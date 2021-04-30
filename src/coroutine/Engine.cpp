#include <afina/coroutine/Engine.h>

#include <setjmp.h>
#include <stdio.h>
#include <string.h>
#include <cassert>

#include <cstring>

namespace Afina {
namespace Coroutine {

void Engine::Store(context &ctx)
{
    char c;
    if (&c <= ctx.Low)
    {
        ctx.Low = &c;
    }
    else
    {
        ctx.Hight = &c;
    }

    std::size_t stack_size = ctx.Hight - ctx.Low;
    if (std::get<1>(ctx.Stack) < stack_size || 2 * std::get<1>(ctx.Stack) > stack_size)
    {
        DeleteStack(ctx);
        std::get<0>(ctx.Stack) = new char[stack_size];
        std::get<1>(ctx.Stack) = stack_size;
    }
    std::memcpy(std::get<0>(ctx.Stack), ctx.Low, stack_size);
}

void Engine::Restore(context &ctx)
{
    char c;
    if (&c >= ctx.Low && &c <= ctx.Hight)
    {
        Restore(ctx);
    }
    std::size_t mem_restore = ctx.Hight - ctx.Low;
    std::memcpy(ctx.Low, std::get<0>(ctx.Stack), mem_restore);
    cur_routine = &ctx;
    longjmp(ctx.Environment, 1);
}

void Engine::yield()
{
    // switch to alive coro
    context* cur = alive;
    if (cur != nullptr)
    {
        if (cur == cur_routine)
        {
            cur = cur->next;
        }
    }
    if (cur != nullptr)
    {
        sched(cur);
    }
}

void Engine::sched(void *coroutine)
{
    if (coroutine == cur_routine)
    {
        return;
    }
    else if (coroutine == nullptr)
    {
        yield();
        return;
    }
    assert(cur_routine != nullptr);
    if (cur_routine != idle_ctx)
    {
        if (setjmp(cur_routine->Environment) > 0)
        {
            // start coro
            return;
        }
        Store(*cur_routine); // save stack
    }
    cur_routine = (context*) coroutine;
    Restore(*cur_routine); // switch to coro
}

void Engine::block(void* coro) {

    context* blocking = (context *)coro;
    if (blocking == nullptr) {
        blocking = cur_routine;
    }
    // remove from alive
    if (alive == blocking) {
        alive = alive->next;
    }
    if (blocking->next != nullptr)
    {
        blocking->next->prev = blocking->prev;
    }
    if (blocking->prev != nullptr)
    {
        blocking->prev->next = blocking->next;
    }
    // add to blocked
    if (blocked == nullptr)
    {
        blocked = blocking;
        blocked->next = nullptr;
        blocked->prev = nullptr;
    }
    else
    {
        blocking->prev = nullptr;
        blocked->prev = blocking;
        blocking->next = blocked;
        blocked = blocking;
    }
    if (blocking == cur_routine)
    {
        Restore(*idle_ctx);
    }
}

void Engine::unblock(void *coro)
{
    context* unblocking = (context *)coro;
    if (unblocking == nullptr) {
        return;
    }
    // remove from blocked
    if (blocked == unblocking) {
        blocked = blocked->next;
    }
    if (unblocking->next != nullptr)
    {
        unblocking->next->prev = unblocking->prev;
    }
    if (unblocking->prev != nullptr)
    {
        unblocking->prev->next = unblocking->next;
    }
    // add to alive
    if (alive == nullptr)
    {
        alive = unblocking;
        alive->next = nullptr;
        alive->prev = nullptr;
    }
    else
    {
        unblocking->prev = nullptr;
        alive->prev = unblocking;
        unblocking->next = alive;
        alive = unblocking;
    }
}
Engine::~Engine()
{
    while (alive != nullptr)
    {
        context* tmp = alive;
        delete tmp;
        alive = alive->next;
    }
    while (blocked != nullptr)
    {
        context* tmp = blocked;
        delete tmp;
        blocked = blocked->next;
    }
}

void Engine::DeleteStack(context& ctx)
{
    delete[] std::get<0>(ctx.Stack);
}

} // namespace Coroutine
} // namespace Afina
