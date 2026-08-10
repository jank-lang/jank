# analyze/mismatched-if-types

This happens when the `then` branch and `else` branch of an `if` produce native C++ values with incompatible types. jank needs both branches to produce a compatible result type for the full expression.

This usually is not a problem for ordinary jank objects. It most often appears when native C++ values are involved.

## Mitigations
Change the branches so they produce compatible types. jank can handle some implicit conversions and trait conversions automatically. If that is not enough, wrap both branches in a common type such as `std::variant`.
