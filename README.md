# Group 6 — University Library Borrowing, Fines and Book-State Tracker

## Group Members

| Name | Registration Number |
|---|---|
| Raymond Rono | C026-01-0909/2025 |
| Meshack Setek | C026-01-0974/2025 |
| Kelvin Kimutai | C026-01-0962/2025 |

**File:** `library_system.cpp`
**Language:** C++17
**Compile:** `g++ -std=c++17 -Wall -o library_system library_system.cpp`
**Run:** `./library_system`

## 1. What the program does

Simulates a library's book catalogue, student borrowing, returns, and overdue
fines, while deliberately exposing how names, bindings, scope, lifetime,
aliasing, and heap storage behave in a running C++ program (per the lab brief
for Group 6).

- 8 books seeded in `library` (a 9th, `9009`, is added later via the heap
  experiment — see §3.C).
- 5 successful borrow transactions, 2 rejected borrows (unknown accession,
  no copies left), 3 successful returns, 1 rejected return (mismatched
  student ID) — well past the "5 borrowing/return transactions" minimum.
- Constants `MAX_LOAN_DAYS` and `CHARGE_PER_DAY` drive due dates and fines.

## 2. One-page concept map (concept → exact code location)

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

## 3. Sample test runs (from an actual execution)

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

## 4. Variable/diagram table (six key variables — name, type, scope, lifetime)

| Name | Type | Scope | Lifetime |
|---|---|---|---|
| `library` | `vector<Book>` | Global (file scope) | Whole program |
| `transactionLog` | `vector<Transaction>` | Global (file scope) | Whole program |
| `totalLoans` | `int` (static local) | Local to `borrowBook()` | Whole program |
| `searchIndex` | `int` (local) | Local to `searchBook()` | One call of `searchBook()` |
| `dynamicBook` | `Book*` (heap pointer) | Local to `main()` | From `new` until explicit `delete` |
| `ref1`, `ref2` | `Book&` (references) | Local to `demonstrateAliasing()` | Duration of that function call (the *object* they alias, `library[0]`, outlives them) |

## 5. Answering the lab's reflection questions

- **Do two same-named variables in different scopes share memory?**
  No. `basicPay`-style shadowing shown by `ref1`/`ref2` vs. any unrelated local
  named similarly elsewhere would each get independent storage — same name,
  different location, resolved by whichever scope is currently active.
- **If two different names share one location, what relationship exists?**
  They are **aliases** — exactly what `ref1` and `ref2` demonstrate: two
  names, one object, so a change via either name is visible via both.
