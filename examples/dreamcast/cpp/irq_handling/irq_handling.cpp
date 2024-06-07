/* KallistiOS ##version##

   examples/dreamcast/cpp/irq_handling/irq_handling.cpp

   Copyright (C) 2024 Falco Girgis

*/

#include <concepts>
#include <functional>
#include <type_traits>
#include <print>
#include <format>
#include <string>
#include <cfloat>
#include <cstdlib>
#include <atomic>
#include <kos.h>

template <> 
struct std::formatter<irq_t>: formatter<const char *> {
    auto format(irq_t &code, format_context &ctx) const {
        switch(code) {
            case EXC_FPU: 
                return formatter<const char *>::format("EXC_FPU",         ctx);
            case EXC_GENERAL_FPU:
                return formatter<const char *>::format("EXC_GENERAL_FPU", ctx);
            case EXC_SLOT_FPU:
                return formatter<const char *>::format("EXC_SLOT_FPU",    ctx);
            default:
                return formatter<const char *>::format("UNKNOWN",         ctx);
        }
    }
};

namespace irq {
    struct context: public irq_context_t {};
    static_assert(sizeof(context) == sizeof(irq_context_t),
                  "Could not safely inherit iq_context_t in C++'s irq::context!");

    template<class F> 
    concept handler = 
        std::invocable<F, irq_t, context *> &&
        std::same_as<std::invoke_result_t<F, irq_t, context *>, bool>;

    using erased_handler = std::function<bool(irq_t, context *)>;

    namespace {
        void cpp_handler_adapter(irq_t code, irq_context_t *ctx, void* data) {
            auto *handler = static_cast<erased_handler *>(data);
            irq_handle_int((*handler)(code, static_cast<context *>(ctx)));
        }
    }

    template<irq_t... codes>
    bool set_handler(handler auto &&callback) {
        auto* erased = new erased_handler(std::forward<decltype(callback)>(callback));
        return (irq_set_handler(codes, cpp_handler_adapter, erased) & ...);
    }

    bool set_global_handler(handler auto &&callback) {
        auto* erased = new erased_handler(std::forward<decltype(callback)>(callback));
        return irq_set_global_handler(cpp_handler_adapter, erased);
    }
}

struct handler_ctrl {
    std::atomic<size_t> called_count  = 0;
    std::atomic<bool>   should_handle = true;
};
 
template<irq_t... codes>
auto meta_handler(std::string name, handler_ctrl& ctrl, auto &&handler)
    requires std::invocable<std::remove_reference_t<decltype(handler)>,
                            irq_t, irq::context *> 
{
    return ([&ctrl, name=std::move(name), &handler]
            (irq_t code, irq::context *ctx) 
    {
        ++ctrl.called_count;
        std::println("Caught exception: {} from {}!", code, name);
        std::invoke(std::forward<decltype(handler)>(handler), code, ctx);
        return static_cast<bool>(ctrl.should_handle);
    });
}

static void divide_by_zero_exception() {
    // Enable divide-by-zero exceptions in the FPU status register.
    __builtin_sh_set_fpscr(__builtin_sh_get_fpscr() | 0b111100000000);
    {   //RIP FPU!
        volatile double d = 0.0, c = 0.0, e = d / c; (void)e;
    }
}

int main(int argc, char* argv[]) {
    handler_ctrl fpu_ctrl, unhandled_ctrl, global_ctrl;
    bool success = true;

    irq::set_global_handler(
        meta_handler("Global Handler", global_ctrl, [](irq_t, irq::context *ctx) {
            ctx->pc += 2;
        }));

    irq::set_handler<EXC_FPU, EXC_GENERAL_FPU, EXC_SLOT_FPU>(
        meta_handler("Single Handler", fpu_ctrl, [](irq_t, irq::context *ctx) {
            ctx->pc += 2;
        }));

    irq::set_handler<EXC_UNHANDLED_EXC>(
        meta_handler("Unhandled Handler", unhandled_ctrl, [](irq_t, irq::context *ctx) {
            ctx->pc += 2;
        }));
    
    std::println("Testing out accepting the exception in the global handler...");
    divide_by_zero_exception();

    if(global_ctrl.called_count != 1 || fpu_ctrl.called_count || unhandled_ctrl.called_count) {
        std::println("Global handler failed to accept exception!");
        success = false;
    }

    std::println("\nTesting out accepting the exception in the single handler...");
    global_ctrl.should_handle = false;
    fpu_ctrl.should_handle    = true;
    divide_by_zero_exception();

    if(global_ctrl.called_count != 2 || fpu_ctrl.called_count != 1 || unhandled_ctrl.called_count) {
        std::println("Single handler failed to accept exception!");
        success = false;
    }

    std::println("\nTesting out accepting the exception in the unhandled handler...");
    fpu_ctrl.should_handle       = false;
    unhandled_ctrl.should_handle = true;
    divide_by_zero_exception();

    if(global_ctrl.called_count != 3 || fpu_ctrl.called_count != 2 || unhandled_ctrl.called_count != 1) {
        std::println("Unhandled handler failed to accept exception!");
        success = false;
    }
    
    if(success) { 
        std::println(stdout, "\n========== C++ IRQ HANDLER TEST: PASSED! ===========");
        return EXIT_SUCCESS;
    } else {
        std::println(stderr, "\n!!!!!!!!!! C++ IRQ HANDLER TEST: FAILED! !!!!!!!!!!");
        return EXIT_FAILURE;
    }
}
