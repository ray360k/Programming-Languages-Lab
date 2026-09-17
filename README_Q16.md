# Question 16 — Producer-Consumer Food Distribution System (Lua Coroutines)

## Group Members

| Name | Registration Number |
|---|---|
| Raymond Rono | C026-01-0909/2025 |
| Meshack Setek | C026-01-0974/2025 |
| Kelvin Kimutai | C026-01-0962/2025 |

**File:** `producer_consumer.lua`
**Language:** Lua 5.4
**Run:** `lua5.4 producer_consumer.lua` (or `lua producer_consumer.lua`)

## 1. What the program does

Farmers in Nyandarua produce potato consignments (varying sizes) for three
urban markets — Nyeri, Nakuru and Nairobi — each with its own limited
storage capacity. A single **producer coroutine** generates 7 consignments
one at a time via `coroutine.yield()`. A scheduler resumes the producer,
receives each consignment, and decides in real time which market(s) should
receive it, including deliberately forcing a consignment (#7, 250 bags)
that is too large for any single market and even exceeds the combined
remaining capacity — to exercise the split and insufficient-storage paths.

## 2. Requirement → code map

| Task | Where it appears | What it shows |
|---|---|---|
| **(a) Produce consignments incrementally using `yield()`** | `producer` coroutine (the `for` loop calling `coroutine.yield(c.id, c.size)`) | Each `coroutine.resume(producer)` call advances the loop by exactly one consignment; the loop's position and the `consignments` list are preserved automatically between resumes — that is the coroutine's persistent state. |
| **(b) Separate storage capacities for the three markets** | `markets` table (`Nyeri Market`, `Nakuru Market`, `Nairobi Market`) | Each entry holds its own independent `capacity` field, updated only by the allocation functions. |
| **(c) Determine dynamically which market receives each consignment** | `selectBestFitMarket()` | Best-fit heuristic: among markets that can hold the whole consignment, picks the one that would be left with the smallest leftover space, decided fresh for every consignment as sizes are only known at run time. |
| **(d) Handle a consignment that cannot fit completely into any one market** | `splitAcrossMarkets()` | Sorts markets by remaining capacity (largest first) and fills them in turn until the consignment is placed or all markets are full; any bags still unplaced are reported explicitly (see consignment #7 in the sample run). |
| **(e) Defend the allocation strategy (efficiency and fairness)** | See §4 below and the in-code comment above `selectBestFitMarket()` | Best-fit is efficient because it avoids "wasting" a large market's capacity on a small consignment when a tighter-fitting market exists, leaving more large-capacity headroom for future big consignments. It is fair in the sense that the decision depends only on the numbers (capacity vs. size) each round, never on a fixed market ordering or favouritism, so every market gets consignments purely on merit of fit. |
| **Correct `coroutine.status()` / `resume()` use, never resuming a dead coroutine** | Main `while` loop | Loops `while coroutine.status(producer) ~= "dead"`; checks the boolean success flag from `coroutine.resume()` before touching its other return values, and distinguishes "yielded" from "returned/finished" by re-checking `coroutine.status()` right after each resume — so a dead coroutine is never resumed again. |

## 3. Sample run highlights

- Consignments #1–#6 are each placed whole into whichever market gives the
  tightest fit (best-fit bin packing).
- Consignment #7 (250 bags) does not fit whole anywhere: the scheduler
  splits it — 40 bags to Nairobi, 5 to Nyeri — and correctly reports that
  **205 bags could not be allocated**, since total remaining county-wide
  capacity (45 bags) was less than the consignment size.
- After the producer coroutine returns, its status is confirmed `dead` and
  the scheduler stops resuming it, then prints the final capacity of all
  three markets (0 bags remaining each, in this run).

## 4. Efficiency and fairness discussion (part e)

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
