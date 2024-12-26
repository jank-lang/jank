// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #include "doctest/doctest.h"
// #include "jank/analyze/processor.hpp"
// #include "jank/runtime/context.hpp"
// #include "jank/runtime/obj/symbol.hpp"
//
// #include <memory>
//
// // using namespace jank::runtime;
// namespace jank::analyze {
//     TEST_SUITE("processor") {
//         TEST_CASE("analyze_def with valid input") {
//             runtime::context rt_ctx;
//             processor proc(rt_ctx);
//
//             // Create a valid def form: (def x 42)
//             auto sym = make_box<obj::symbol>("x");
//             auto val = 42ll;
//   //           auto const wrapped(
//   // make_box<obj::persistent_list>(std::in_place, make_box<obj::symbol>("fn*"), args, call));
//
//             auto const def_form ( make_box<obj::persistent_list>(std::in_place,
//                                                            make_box<obj::symbol>("def"), // First element: "def" symbol
//                                                            {sym}, // Second element: variable symbol
//                                                            val // Third element: value
//             ));
//
//             auto frame = make_box<local_frame>(local_frame::frame_type::root, rt_ctx, none);
//             auto result = proc.analyze(def_form, frame, expression_position::value, none, true);
//
//             CHECK(result.is_ok());
//             auto expr = result.expect_ok();
//             CHECK(expr != nullptr);
//         }
//
//         TEST_CASE("analyze_def with missing elements") {
//             runtime::context rt_ctx;
//             processor proc(rt_ctx);
//
//             // Create an invalid def form: (def x)
//             auto sym = make_box<obj::symbol>("x");
//             auto def_form = make_box<obj::persistent_list>(std::in_place,
//                                                            make_box<obj::symbol>("def"),
//                                                            sym
//             );
//
//             auto frame = make_box<local_frame>(local_frame::frame_type::root, rt_ctx, none);
//             auto result = proc.analyze(def_form, frame, expression_position::value, none, true);
//
//             CHECK(result.is_err());
//         }
//
//         TEST_CASE("analyze_if with valid input") {
//             runtime::context rt_ctx;
//             processor proc(rt_ctx);
//
//             // Create a valid if form: (if true 1 0)
//             auto condition = make_box<obj::boolean>(true);
//             auto then_expr = 1ll;
//             auto else_expr = 0ll;
//             auto if_form = make_box<obj::persistent_list>(std::in_place,
//                                                           make_box<obj::symbol>("if"),
//                                                           condition,
//                                                           then_expr,
//                                                           else_expr
//             );
//
//             auto frame = make_box<local_frame>(local_frame::frame_type::root, rt_ctx, none);
//             auto result = proc.analyze(if_form, frame, expression_position::value, none, true);
//
//             CHECK(result.is_ok());
//             auto expr = result.expect_ok();
//             CHECK(expr != nullptr);
//         }
//
//         TEST_CASE("analyze_if with missing else branch") {
//             runtime::context rt_ctx;
//             processor proc(rt_ctx);
//
//             // Create an invalid if form: (if true 1)
//             auto condition = make_box<obj::boolean>(true);
//             auto then_expr = 1ll;
//             auto if_form = make_box<obj::persistent_list>(std::in_place,
//                                                           make_box<obj::symbol>("if"),
//                                                           condition,
//                                                           then_expr
//             );
//
//             auto frame = make_box<local_frame>(local_frame::frame_type::root, rt_ctx, none);
//             auto result = proc.analyze(if_form, frame, expression_position::value, none, true);
//
//             CHECK(result.is_ok()); // Should allow missing else as it defaults to nil.
//         }
//     }
// }
