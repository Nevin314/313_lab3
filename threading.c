#include <threading.h>
#include <stdint.h>

uint8_t find_next_free(uint8_t idx)
{
    uint8_t next = idx;

    next++;
    next %= NUM_CTX;

    while (next != idx)
    {
        if (contexts[next].state == INVALID)
        {
            break;
        }
        next++;
        next %= NUM_CTX;
    }
    if (next == idx)
        return 255;
    return next;
}

uint8_t find_next_valid(uint8_t idx)
{
    uint8_t next = idx;
    next++;
    next %= NUM_CTX;

    while (next != idx)
    {
        if (contexts[next].state == VALID)
        {
            break;
        }
        next++;
        next %= NUM_CTX;
    }
    if (next == idx)
        return 255;
    return next;
}

void t_init(void)
{
    // TODO
    /* set up data structures/variables needed */

    for (int i = 0; i < NUM_CTX; i++)
    {
        contexts[i].state = INVALID;
    }
    getcontext(&contexts[0].context);
    contexts[0].state = VALID;
    current_context_idx = 0;
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)
{
    // TODO

    uint8_t next_cont = find_next_free(current_context_idx);
    if (next_cont == 255)
        return 1;
    getcontext(&contexts[next_cont].context);

    contexts[next_cont].context.uc_stack.ss_sp = (char *)malloc(STK_SZ);
    contexts[next_cont].context.uc_stack.ss_size = STK_SZ;
    contexts[next_cont].context.uc_stack.ss_flags = 0;
    contexts[next_cont].context.uc_link = 0;

    makecontext(&contexts[next_cont].context, (ctx_ptr)foo, 2, arg1, arg2);
    contexts[next_cont].state = VALID;

    return 0;
    /* create a task with a function, this should use makecontext somehow */
}

int32_t t_yield()
{
    // TODO
    getcontext(&contexts[current_context_idx].context);

    uint8_t next_cont = find_next_valid(current_context_idx);

    if (next_cont == current_context_idx || next_cont == 255)
    {
        return -1;
    }

    int prev = current_context_idx;
    current_context_idx = next_cont;
    // swapcontext(&contexts[current_context_idx].context, &contexts[next_cont].context);
    swapcontext(&contexts[prev].context, &contexts[next_cont].context);

    int cnt = 0;
    for (int i = 0; i < NUM_CTX; i++)
    {
        if (i != current_context_idx && contexts[i].state == VALID)
        {
            cnt++;
        }
    }

    return cnt;
    /* transfer control to the next context, use swapcontext */
}

void t_finish()
{
    // TODO
    /* what does this do? */
    contexts[current_context_idx].state = DONE;
    free(contexts[current_context_idx].context.uc_stack.ss_sp);

    uint8_t nxt = find_next_valid(current_context_idx);
    if (nxt == 255)
        exit(0);

    uint8_t prev = current_context_idx;
    current_context_idx = nxt;
    swapcontext(&contexts[prev].context, &contexts[current_context_idx].context);
}
