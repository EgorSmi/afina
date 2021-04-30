#include "SimpleLRU.h"

namespace Afina {
namespace Backend {


void SimpleLRU::drop_tail()
{
  lru_node* last = _lru_tail; // node to delete
  _cur_size -= last->key.size() + last->value.size();
  _lru_index.erase(last->key); // delete from map
  if (_lru_head.get() == last)
  {
    // one element in the List
    _lru_head.reset(); // destroy object; same as _lru_head = std::move(nullptr)
  }
  else
  {
    _lru_tail = last->prev;
    _lru_tail->next.reset(); // delete object managed by unique_ptr
  }
}

void SimpleLRU::transfer_fresh(lru_node* node)
{
  // list has at least one node
  if (node != _lru_head.get())
  {
    lru_node* prev = node->prev;
    lru_node* next = node->next.get();
    if (next != nullptr)
    {
      // _lru_head.get() == nullptr after std::move(_lru_head) !!!!! thx
      std::unique_ptr<lru_node> tmp = std::move(_lru_head);
      tmp.get()->prev = node;
      _lru_head = std::move(prev->next);
      prev->next = std::move(node->next);
      node->next = std::move(tmp);
      next->prev = prev;
    }
    else
    {
      // transfer tail
      _lru_tail = prev;
      _lru_head.get()->prev = node;
      node->next = std::move(_lru_head); // here must be std::move!!!!
      _lru_head = std::move(prev->next); // because _lru_head.get() == nullptr
    }
  }
}

void SimpleLRU::delete_node(lru_node* node)
{
  if (node == _lru_head.get())
  {
    if (node->next.get() == nullptr)
    {
      // only head in List
      _lru_tail = nullptr;
      _lru_head.reset();
    }
    else
    {
      _lru_head.get()->next.get()->prev = nullptr;
      _lru_head = std::move(_lru_head.get()->next);
    }
  }
  else
  {
    lru_node* prev = node->prev;
    lru_node* next = node->next.get();
    if (prev != nullptr)
    {
      prev->next = std::move(node->next);
    }
    if (next != nullptr)
    {
      next->prev = node->prev;
    }
    else
    {
      // need to delete tail
      _lru_tail = prev;
    }
  }
}

void SimpleLRU::insert(lru_node* node)
{
  // insert new node
  if (_lru_head.get() == nullptr)
  {
    // no nodes
    _lru_tail = node;
  }
  else
  {
    _lru_head.get()->prev = node;
  }
  node->prev = nullptr;
  node->next = std::move(_lru_head); // here must be std::move!!!!
  //_lru_head is not owner of the object and _lru_head.get() == nullptr
  _lru_head.reset(node);
}

void SimpleLRU::change_pair(lru_node& node, const std::string& new_value)
{
  int new_size = new_value.size() - node.value.size(); // here must be int not size_t
  transfer_fresh(&node); // prototype: transfer_fresh(lru_node*)
  while(_cur_size + new_size > _max_size)
  {
    drop_tail();
  }
  node.value = new_value;
  _cur_size += new_size;
}

void SimpleLRU::create_pair(const std::string& key,const std::string& value)
{
  std::size_t new_size = key.size() + value.size();
  while (_cur_size + new_size > _max_size)
  {
    drop_tail();
  }
  _cur_size += new_size;
  lru_node* new_node = new lru_node{key, value, nullptr, nullptr};
  insert(new_node);
  // update Index
  _lru_index.insert(std::make_pair(std::reference_wrapper<const std::string>(new_node->key), // here new_node->key!!! not key
                       std::reference_wrapper<lru_node>(*new_node)));
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
  if (key.size() + value.size() > _max_size)
  {
    // this record size is forbidden
    return false;
  }
  auto it = _lru_index.find(key);
  if (it == _lru_index.end())
  {
    // new pair
    create_pair(key, value);
  }
  else
  {
    // key has already existed
    change_pair(it->second.get(), value);
  }
  return true;

}
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
  if (key.size() + value.size() > _max_size)
  {
    // this record size is forbidden
    return false;
  }
  auto it = _lru_index.find(key);
  if (it == _lru_index.end())
  {
    // new pair
    create_pair(key, value);
  }
  else
  {
    // key has already existed
    return false;
  }
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
  if (key.size() + value.size() > _max_size)
  {
    // this record size is forbidden
    return false;
  }
  auto it = _lru_index.find(key);
  if (it != _lru_index.end())
  {
    change_pair(it->second.get(), value);
  }
  else
  {
    return false;
  }
  return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
  auto it = _lru_index.find(key);
  if (it != _lru_index.end())
  {
    lru_node* node = &(it->second.get()); // check the order of erase!!!! so stupid.......
    _lru_index.erase(key);
    _cur_size -= key.size() + node->value.size();
    delete_node(node);
    return true;
  }
  return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value)
{
  auto it = _lru_index.find(key);
  if (it != _lru_index.end())
  {
    transfer_fresh(&(it->second.get()));
  }
  else
  {
    return false;
  }
  value = it->second.get().value;
  return true;
}

} // namespace Backend
} // namespace Afina
