# Error Message Style Guide
This document defines the standard for writing error messages in jank. It exists so
that anyone adding a new diagnostics has a clear, checkable set of rules to
follow, rather than guessing.

## Philosophy
An error message is not a log line. It's written for a person who is stuck and
wants to get unstuck. Every message should read as if a knowledgeable colleague
is looking over the user's shoulder and explaining, calmly and precisely, what
went wrong.

Three questions every error should let the reader answer:

1. **What** is wrong?
2. **Why** is it wrong (which rule was violated)?
3. **How** to fix it?

If a message can't answer the first two, it isn't finished yet.

---

## Content rules
These govern what the message must actually communicate, independent of
grammar.

### 1. Say what was expected *and* what was found
Say what the value was invalid *relative to* and include all relevant context.

```text
✗ index out of bounds: 7
✓ Index `7` is out of bounds for a `persistent_vector` of length `3`.

✗ not nameable: 42
✓ Objects of type `integer` do not have a name.
```

### 2. Prefer specific words over vague ones
Words like *invalid*, *bad*, *wrong*, or *not supported* are fine as the verb
of the sentence, but must always be followed by the specific rule that was
broken. Never leave them standing alone.

```text
✗ invalid argument
✓ The first argument to `subs` must be a string, not `nil`.
```

### 3. Never blame the user
State the constraint objectively. The tone should be identical whether the
mistake was a typo or a genuine misunderstanding of the language.

```text
✗ You can't take a namespace by value.
✓ Taking a C++ namespace by value is not permitted.
```

---

## Mechanical rules
These govern grammar and formatting, and should be true of *every* message
without exception.

### 4. Every message must be a complete sentence
Capitalized, ends in a period. No fragments.

```text
✗ expected persistent_string, got boolean
✓ UUID creation requires a `persistent_string`, not a `boolean`.
```

### 5. Wrap types, symbols, keywords, and identifiers in backticks
This applies to function and variable names, C++ type names, language keywords
like `def`, parts of syntax, and literal values referenced as names, like `nil`.

```text
✓ The `.` suffix for constructors may only be used on types. In this case,
  `{}` is a value of type `{}`.
```

### 6. Describe the state of the world, not the compiler's action
Prefer present tense, third person, stating a fact about the user's code.
Don't narrate what the compiler tried and failed to do.

```text
✗ Could not find `foo`.
✗ Failed to resolve `foo`.
✓ `foo` is not defined.
```

Exception: when the *process* itself is the useful information (e.g. a macro
expansion that recursed too deeply), past tense describing what happened is
correct. The rule is "describe the world," not "always use present tense."

### 7. One sentence, one fact
Use simple sentences, each with their own nugget of information. If information
is packed too densely in one sentence, people will miss important bits.

```text
✗ The `.` suffix was used after `{}`, which is a value of type `{}`, but constructors may only be used on types.
✓ The `.` suffix for constructors may only be used on types. In this case,
  `{}` is a value of type `{}`.
```

### 8. Don't render arbitrary values
If you have an `object_ref`, don't blindly get a `to_code_string` and put it
into the message. The object could be an enormous collection or an infinite
sequence. Instead, use the object type. If you know the type and size of the
object, you can render it.

## Quick checklist for review
Before merging a new or changed error message, confirm:

- [ ] Complete sentence, capitalized, with correct grammar and a period.
- [ ] Every type/symbol/identifier/syntax is backticked.
- [ ] States both what was expected and what was found, if both are known.
- [ ] No blame language ("you forgot," "you can't").
- [ ] Present tense, describes the code's state, not the compiler's process
      unless the process itself is the useful information.
- [ ] One fact per sentence, with clear, simple sentences.
