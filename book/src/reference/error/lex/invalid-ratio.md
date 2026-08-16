# lex/invalid-ratio

This happens when a ratio literal is not written as two integers separated by `/`. Decimal points are not allowed. Scientific notation is not allowed. Arbitrary-radix notation is not allowed. Non-integer denominators are not allowed.

## Mitigations
Write the ratio as `numerator/denominator` with integers on both sides. If you want a decimal value, use a decimal number instead.
