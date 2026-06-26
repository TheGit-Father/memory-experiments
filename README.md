# memory-experiments


#this is an edit i am making for a missed day 22june

#this is edit for 23june same day

#edit dated june 24

#edit dated june 25

#edit dated june 26(on 27)

Five small C programs that each deliberately corrupt memory in a
different way, built and run both **with** and **without**
AddressSanitizer (`-fsanitize=address`). The point isn't the bugs
themselves — it's seeing how the *same* bug behaves invisibly under a
plain build versus loudly under ASan, and connecting each one back to
the stack/heap memory model (CS:APP Chapter 3, §3.7 and §3.10).

All programs were compiled with `gcc 13.3.0` on Ubuntu 24.04 (x86-64).
Exact addresses and shadow-byte values will differ on your machine —
the error *types* and line numbers will not.

## A note on compiler warnings

If you compile with `-Wall -Wextra`, gcc will warn about two of these
on its own, *before* you even run them:

- `05_use_after_free.c`: `-Wuse-after-free` — gcc's static analysis
  catches this particular bug pattern at compile time. Worth noting:
  this is a relatively recent gcc capability and won't catch every
  use-after-free (e.g. across function boundaries, or through
  aliased pointers) — it's not a substitute for ASan, just a nice
  first line of defense for the simple case.
- `04_off_by_one_overflow.c`: `-Wunused-but-set-variable` on `name` —
  harmless; `name` is written but its final value is never read
  (the bug is in the *writing*, which is the point).

## How to build and run each one

```bash
# Plain build (undefined behavior, no detection)
gcc -g -fno-stack-protector -o prog 0N_name.c
./prog

# ASan build (catches the bug precisely)
gcc -g -fsanitize=address -fno-omit-frame-pointer -o prog_asan 0N_name.c
./prog_asan
```

`-fno-stack-protector` is used for the plain builds so gcc's own
stack-canary defense doesn't mask the bug — these are meant to show
*raw* undefined behavior, not gcc's mitigations. `-g` keeps debug
symbols so ASan can print file/line numbers.

---

## 1. `01_stack_overflow_strcpy.c` — classic stack buffer overflow

An 8-byte local array (`char buf[8]`) on the stack frame of
`vulnerable()` is overflowed with a 24-byte `strcpy()`. This is the
textbook case from CS:APP §3.7.4 (locals live on the stack) combined
with §3.10.3 (out-of-bounds writes corrupting frame state).

| Build | Result |
|---|---|
| Plain | `Segmentation fault` (exit code 139) — crashes when corrupted stack state is used, no explanation of *why* |
| ASan | Reports `stack-buffer-overflow`, names the exact variable (`'buf'`), the exact line (`01_stack_overflow_strcpy.c:18`), and that it was a 25-byte WRITE into an 8-byte object |

**Takeaway:** without ASan, you get a bare crash and have to guess. With it, the tool points at the array and the line in one shot.

---

## 2. `02_heap_overflow_malloc.c` — heap buffer overflow

`malloc(5 * sizeof(int))` allocates 20 bytes on the **heap**, then a
loop writes 15 ints (indices 0–14) into a 5-int allocation. Contrast
with program 1: this corrupts heap allocator metadata, not a stack
return address — a different memory region with different
consequences (CS:APP §3.7.1 distinguishes stack from heap; the heap
isn't covered in depth until Ch. 9, but the contrast is the point
here).

| Build | Result |
|---|---|
| Plain | Runs to completion silently, `data[0] = 0` — no visible symptom this run, but the heap is corrupted (undefined behavior may surface later, elsewhere, unpredictably) |
| ASan | Reports `heap-buffer-overflow`, says exactly **"0 bytes after 20-byte region"**, and shows both the bad WRITE and the original `malloc` call site |

**Takeaway:** this is the dangerous case — the plain build looks completely fine. The bug is real but silent. This is exactly why heap overflows are hard to find without tooling.

---

## 3. `03_overwrite_return_address.c` — corrupting the saved return address

A 16-byte stack buffer is overflowed with a 200-byte string of `'A'`
characters — large enough to plausibly reach past the buffer, past
saved `%rbp`, into the saved return address that `call` pushed onto
the stack (CS:APP §3.7.2, Figure 3.25/3.40). When `vulnerable()`
executes `ret`, the corrupted return address is popped into the
program counter.

| Build | Result |
|---|---|
| Plain | `Segmentation fault` — the corrupted return address (built from `0x41414141...`, the ASCII of repeated `'A'`) points to unmapped memory, so the jump faults |
| ASan | Reports `stack-buffer-overflow` at the `strcpy` call itself, before `ret` is even reached — catches the cause, not just the eventual crash |

**Takeaway:** this is the mechanism behind real-world stack-smashing exploits — overwrite the return address, redirect control flow. ASan catches it at the write; without ASan you only see the *symptom* (a crash potentially far from the actual bug, at the `ret` instruction).

---

## 4. `04_off_by_one_overflow.c` — a one-byte overflow

A classic `<=` vs `<` loop bug: a 5-byte array `name[5]` is written 6
times (indices 0 through 5 inclusive). Only **one byte** goes out of
bounds. A neighboring stack variable `marker` is included to show
where that stray byte can land.

| Build | Result |
|---|---|
| Plain | Runs cleanly, `marker = "SAFE"` — unchanged this run. Compiler-inserted padding (CS:APP §3.9.3, data alignment) silently absorbed the stray byte |
| ASan | Reports `stack-buffer-overflow`, **size 1** WRITE, names `'name'` as the overflowed object and even lists `'marker'` as the adjacent frame object |

**Takeaway:** this is the most realistic bug of the five — the kind that passes code review and "looks fine" in testing, because undefined behavior doesn't guarantee a visible symptom. It's also the best illustration of why off-by-one errors are so dangerous: nothing about a clean run proves the code is correct.

---

## 5. `05_use_after_free.c` — heap use-after-free

32 bytes are allocated, written, then `free()`'d — and then written
to *again* through the now-dangling pointer. Unlike program 2 (a
write outside the bounds), this write is **inside** the original
bounds but happens **after** the memory was returned to the
allocator. This is the manual-management half of CS:APP §3.7.1's
point: stack memory is reclaimed automatically on return; heap memory
is the programmer's responsibility, and nothing stops you from using
it after freeing it.

| Build | Result |
|---|---|
| Plain | Runs cleanly, prints `"use-after-free!"` as if nothing were wrong — looks completely correct |
| ASan | Reports `heap-use-after-free`, shows the WRITE site, the exact `free()` call site, and the original `malloc()` call site — three-way traceback |

**Takeaway:** like program 2, this is a "silent" bug class — plain execution gives zero indication anything is wrong. This is the most common real-world category ASan/Valgrind exist to catch.

---

## Summary table

| # | Program | Region | Plain build | ASan build |
|---|---|---|---|---|
| 1 | `01_stack_overflow_strcpy.c` | Stack | Segfault | `stack-buffer-overflow`, exact var + line |
| 2 | `02_heap_overflow_malloc.c` | Heap | Silent, looks fine | `heap-buffer-overflow`, exact offset |
| 3 | `03_overwrite_return_address.c` | Stack (return addr) | Segfault | `stack-buffer-overflow` caught at write, before crash |
| 4 | `04_off_by_one_overflow.c` | Stack | Silent, looks fine | `stack-buffer-overflow`, size 1 |
| 5 | `05_use_after_free.c` | Heap | Silent, looks fine | `heap-use-after-free`, 3-way traceback |

Three of five plain builds produce **no visible symptom at all** —
that's the core lesson. A program that runs without crashing is not
proof of correctness; it's only proof that this particular run didn't
happen to touch memory in a way that crashed *this time*. Undefined
behavior in C is exactly that: undefined, not "safe as long as it
doesn't crash."

## Relevant CS:APP sections

- §3.7.1 The Run-Time Stack — stack frames, %rsp, automatic
  allocation/deallocation
- §3.7.2 Control Transfer — `call`/`ret`, the return address
- §3.7.4 Local Storage on the Stack — where local variables live
- §3.9.3 Data Alignment — why off-by-one overflows can land in padding
- §3.10.3 Out-of-Bounds Memory References and Buffer Overflow — the
  core reference for programs 1, 3, 4
- §3.10.4 Thwarting Buffer Overflow Attacks — ASLR, stack protector
  (the defenses these programs intentionally bypass with
  `-fno-stack-protector`)
