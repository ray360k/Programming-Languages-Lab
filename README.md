# CCS Coursework — Combined Submission

## Group Members

| Name | Registration Number |
|---|---|
| Raymond Rono | C026-01-0909/2025 |
| Meshack Setek | C026-01-0974/2025 |
| Kelvin Kimutai | C026-01-0962/2025 |

This repository contains two separate exercises submitted together:

1. [Names, Bindings and Scopes — Group 16: University Library Borrowing, Fines and Book-State Tracker](#1-names-bindings-and-scopes--group-16-university-library-borrowing-fines-and-book-state-tracker) (`library_system.cpp`)
2. [CCS 2105 — Question 16: Producer-Consumer Food Distribution System](#2-ccs-2105--question-16-producer-consumer-food-distribution-system) (`producer_consumer.lua`)

---

## 1. Names, Bindings and Scopes — Group 16: University Library Borrowing, Fines and Book-State Tracker

**File:** `library_system.cpp`
**Language:** C++17
**Compile:** `g++ -std=c++17 -Wall -o library_system library_system.cpp`
**Run:** `./library_system`

### 1.1 What the program does

Simulates a library's book catalogue, student borrowing, returns, and overdue
fines, while deliberately exposing how names, bindings, scope, lifetime,
aliasing, and heap storage behave in a running C++ program.

- 8 books seeded in `library` (a 9th, `9009`, is added later via the heap
  experiment — see §1.3.C).
- 5 successful borrow transactions, 2 rejected borrows (unknown accession,
  no copies left), 3 successful returns, 1 rejected return (mismatched
  student ID) — well past the "5 borrowing/return transactions" minimum.
- Constants `MAX_LOAN_DAYS` and `CHARGE_PER_DAY` drive due dates and fines.

### 1.2 One-page concept map (concept → exact code location)

| Concept | Where it appears | What to look for |
|---|---|---|
| **Named constants** | `MAX_LOAN_DAYS`, `CHARGE_PER_DAY` (top of file) | Bound once at compile time; used in `borrowBook()` and `calculateFine()` instead of magic numbers `14` / `20`. |
| **Variable attributes (name, type, scope, lifetime)** | `Book`, `Transaction` structs; `library`, `transactionLog`, `totalSuccessfulBorrows`, `totalFailedBorrows` | These four globals have **global scope** (visible file-wide) and **static/program lifetime** (exist from program start to program end). |
| **Aliases** | `demonstrateAliasing()` | `Book& ref1 = lib[0]; Book& ref2 = ref1;` — two different *names*, one storage location. Changing `ref1.availableCopies` is immediately observed through `ref2`, proving they refer to the same object, not copies. |
| **References as parameters** | `borrowBook(vector<Book>& lib, ...)`, `returnBook(vector<Book>& lib, ...)` | Passed by reference (`&`) so the *original* vector is modified, not a copy — the formal parameter `lib` is itself an alias for the caller's `library`. |
| **Explicit heap-dynamic storage** | `main()`, block C | `Book* dynamicBook = new Book{...};` allocates on the heap. Released explicitly later with `delete dynamicBook;`. Its allocation/deallocation is entirely programmer-controlled, unlike stack variables. |
| **Lifetime vs a stack-dynamic local** | `searchBook()` vs `dynamicBook` | `searchIndex` inside `searchBook()` is created and destroyed on **every single call**. `dynamicBook` survives across two separate `inspectBook()` calls and is only destroyed by the explicit `delete` — demonstrating a longer, programmer-controlled lifetime. |
| **Scope vs lifetime (static local)** | `static int totalLoans` inside `borrowBook()` | **Scope**: local — the name `totalLoans` cannot be referenced from anywhere outside `borrowBook()`. **Lifetime**: the whole program — its value is retained and incremented across every call, unlike an ordinary local variable which would reset to 0 each time. |
| **Object alive but not visible (2nd scope/lifetime example)** | Comment block "D. Scope vs lifetime: additional example" in `main()` | The `Book` objects stored inside global `library` live for the whole program, but the local `idx`/`searchIndex` variables used inside `searchBook()`/`borrowBook()` to reach them die the instant those functions return — the object outlives the name used to reach it. |
| **Validation / exceptional cases** | `borrowBook()`, `returnBook()` | Rejects: unknown accession number, zero available copies, and a return that doesn't match an open loan for that student. |

### 1.3 Sample test runs (from an actual execution)

- **Successful borrow:** `Amina Cherono` borrows accession `1001` on day 1 → due day 15.
- **Boundary/error — no copies left:** `Amina Cherono` tries to borrow accession `1004`
  again on day 3, but its only copy is already out → `[FAIL] ... no available copies.`
- **Boundary/error — unknown accession:** `Brian Otieno` requests accession `9999`,
  which does not exist → `[FAIL] No book with accession number 9999.`
- **Late return / fine:** `Brian Otieno` returns accession `1002` on day 20
  (due day 15) → 5 days late × `CHARGE_PER_DAY (20)` = **KES 100.00** fine.
- **Boundary/error — mismatched return:** an attempt to return accession `1006`
  under the wrong student ID is rejected because no open loan matches.

Full transaction log, stock table, and overdue-charge summary are printed at
the end of the run (see program output, section "E. FINAL REPORTS").

### 1.4 Variable/diagram table (six key variables — name, type, scope, lifetime)

| Name | Type | Scope | Lifetime |
|---|---|---|---|
| `library` | `vector<Book>` | Global (file scope) | Whole program |
| `transactionLog` | `vector<Transaction>` | Global (file scope) | Whole program |
| `totalLoans` | `int` (static local) | Local to `borrowBook()` | Whole program |
| `searchIndex` | `int` (local) | Local to `searchBook()` | One call of `searchBook()` |
| `dynamicBook` | `Book*` (heap pointer) | Local to `main()` | From `new` until explicit `delete` |
| `ref1`, `ref2` | `Book&` (references) | Local to `demonstrateAliasing()` | Duration of that function call (the *object* they alias, `library[0]`, outlives them) |

### 1.5 Answering the lab's reflection questions

- **Do two same-named variables in different scopes share memory?**
  No. Each gets independent storage — same name, different location,
  resolved by whichever scope is currently active.
- **If two different names share one location, what relationship exists?**
  They are **aliases** — exactly what `ref1` and `ref2` demonstrate: two
  names, one object, so a change via either name is visible via both.

---

## 2. CCS 2105 — Question 16: Producer-Consumer Food Distribution System

**File:** `producer_consumer.lua`
**Language:** Lua 5.4
**Run:** `lua5.4 producer_consumer.lua` (or `lua producer_consumer.lua`)

### 2.1 What the program does

Farmers in Nyandarua produce potato consignments (varying sizes) for three
urban markets — Nyeri, Nakuru and Nairobi — each with its own limited
storage capacity. A single **producer coroutine** generates 7 consignments
one at a time via `coroutine.yield()`. A scheduler resumes the producer,
receives each consignment, and decides in real time which market(s) should
receive it, including deliberately forcing a consignment (#7, 250 bags)
that is too large for any single market and even exceeds the combined
remaining capacity — to exercise the split and insufficient-storage paths.

### 2.2 Requirement → code map

| Task | Where it appears | What it shows |
|---|---|---|
| **(a) Produce consignments incrementally using `yield()`** | `producer` coroutine (the `for` loop calling `coroutine.yield(c.id, c.size)`) | Each `coroutine.resume(producer)` call advances the loop by exactly one consignment; the loop's position and the `consignments` list are preserved automatically between resumes — that is the coroutine's persistent state. |
| **(b) Separate storage capacities for the three markets** | `markets` table (`Nyeri Market`, `Nakuru Market`, `Nairobi Market`) | Each entry holds its own independent `capacity` field, updated only by the allocation functions. |
| **(c) Determine dynamically which market receives each consignment** | `selectBestFitMarket()` | Best-fit heuristic: among markets that can hold the whole consignment, picks the one that would be left with the smallest leftover space, decided fresh for every consignment as sizes are only known at run time. |
| **(d) Handle a consignment that cannot fit completely into any one market** | `splitAcrossMarkets()` | Sorts markets by remaining capacity (largest first) and fills them in turn until the consignment is placed or all markets are full; any bags still unplaced are reported explicitly (see consignment #7 in the sample run). |
| **(e) Defend the allocation strategy (efficiency and fairness)** | See §2.4 below and the in-code comment above `selectBestFitMarket()` | Best-fit is efficient because it avoids "wasting" a large market's capacity on a small consignment when a tighter-fitting market exists, leaving more large-capacity headroom for future big consignments. It is fair in the sense that the decision depends only on the numbers (capacity vs. size) each round, never on a fixed market ordering or favouritism, so every market gets consignments purely on merit of fit. |
| **Correct `coroutine.status()` / `resume()` use, never resuming a dead coroutine** | Main `while` loop | Loops `while coroutine.status(producer) ~= "dead"`; checks the boolean success flag from `coroutine.resume()` before touching its other return values, and distinguishes "yielded" from "returned/finished" by re-checking `coroutine.status()` right after each resume — so a dead coroutine is never resumed again. |

### 2.3 Sample run highlights

- Consignments #1–#6 are each placed whole into whichever market gives the
  tightest fit (best-fit bin packing).
- Consignment #7 (250 bags) does not fit whole anywhere: the scheduler
  splits it — 40 bags to Nairobi, 5 to Nyeri — and correctly reports that
  **205 bags could not be allocated**, since total remaining county-wide
  capacity (45 bags) was less than the consignment size.
- After the producer coroutine returns, its status is confirmed `dead` and
  the scheduler stops resuming it, then prints the final capacity of all
  three markets (0 bags remaining each, in this run).

### 2.4 Efficiency and fairness discussion (part e)

**Efficiency:** best-fit minimises wasted space per allocation — a market
is never given far more capacity headroom removed than necessary, which
keeps larger markets available for later large consignments rather than
being consumed early by small ones.

**Fairness:** no market is preferred by name, position in the table, or
history of past allocations — the *only* factor is which market's leftover
space would be smallest after accepting the current consignment. Over many
rounds this naturally spreads load according to actual capacity rather than
an arbitrary priority order.

**Trade-off:** best-fit can still leave a market with an awkward, very
small leftover (as happened to Nyeri here, ending with 0 after taking a
5-bag split), which is efficient in this run but could make that leftover
space useless for most future consignment sizes — a classic tension in
bin-packing between short-term efficiency and long-term flexibility.
