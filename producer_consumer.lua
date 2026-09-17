--[[
====================================================================================
 CCS 2105 — QUESTION 16: PRODUCER-CONSUMER FOOD DISTRIBUTION SYSTEM
 Farmers in Nyandarua produce potato consignments for three urban markets.
 A producer coroutine generates consignments incrementally; a scheduler
 decides, for each consignment, which market(s) should receive it, respecting
 each market's remaining storage capacity.

 Run:  lua5.3 producer_consumer.lua      (or: lua producer_consumer.lua)
====================================================================================
--]]

-- ------------------------------------------------------------------------------
-- Market storage state (three markets, each with its own remaining capacity)
-- (b) Separate storage capacities maintained for the three markets.
-- ------------------------------------------------------------------------------
local markets = {
    { name = "Nyeri Market",   capacity = 200 },
    { name = "Nakuru Market",  capacity = 150 },
    { name = "Nairobi Market", capacity = 300 },
}

-- ------------------------------------------------------------------------------
-- (c) Best-fit allocation: among markets that can fully absorb the consignment,
-- pick the one that would be left with the SMALLEST leftover capacity. This
-- keeps large-capacity markets free for future large consignments instead of
-- "wasting" them on small ones -- a classic bin-packing efficiency heuristic,
-- while still being fair in the sense that no market is favoured by name/order,
-- only by how tightly it fits the current consignment.
-- ------------------------------------------------------------------------------
local function selectBestFitMarket(size)
    local bestIndex, bestLeftover = nil, nil
    for i, m in ipairs(markets) do
        if m.capacity >= size then
            local leftover = m.capacity - size
            if bestLeftover == nil or leftover < bestLeftover then
                bestIndex, bestLeftover = i, leftover
            end
        end
    end
    return bestIndex
end

-- ------------------------------------------------------------------------------
-- (d) Handle a consignment that cannot fit completely into any ONE market:
-- split it across multiple markets, largest remaining capacity first, until
-- either the whole consignment is placed or every market is full. Whatever
-- cannot be placed anywhere is reported as unallocated (insufficient total
-- county-wide storage for that round).
-- ------------------------------------------------------------------------------
local function splitAcrossMarkets(id, size)
    print(string.format("  [SPLIT] Consignment #%d (%d bags) does not fit whole in any single market.", id, size))

    -- Work on a list of market indices sorted by capacity, largest first.
    local order = {}
    for i in ipairs(markets) do order[#order + 1] = i end
    table.sort(order, function(a, b) return markets[a].capacity > markets[b].capacity end)

    local remaining = size
    for _, idx in ipairs(order) do
        if remaining <= 0 then break end
        local m = markets[idx]
        if m.capacity > 0 then
            local take = math.min(m.capacity, remaining)
            m.capacity = m.capacity - take
            remaining = remaining - take
            print(string.format("           -> %s takes %d bags (remaining capacity now %d)",
                  m.name, take, m.capacity))
        end
    end

    if remaining > 0 then
        print(string.format("  [WARN]  %d bags from consignment #%d could NOT be allocated -- " ..
              "county-wide storage is insufficient this round.", remaining, id))
    else
        print(string.format("  [OK]    Consignment #%d fully distributed across multiple markets.", id))
    end
end

-- ------------------------------------------------------------------------------
-- Top-level allocation decision for one consignment.
-- ------------------------------------------------------------------------------
local function distributeConsignment(id, size)
    local idx = selectBestFitMarket(size)
    if idx then
        local m = markets[idx]
        m.capacity = m.capacity - size
        print(string.format("  [OK]    Consignment #%d (%d bags) -> %s (best fit, capacity now %d)",
              id, size, m.name, m.capacity))
    else
        splitAcrossMarkets(id, size)
    end
end

-- ------------------------------------------------------------------------------
-- (a) PRODUCER COROUTINE: generates consignments incrementally using yield().
-- Each call to coroutine.resume() advances the producer to the next
-- consignment; its local loop state (the consignments list and current index)
-- is preserved automatically across yields -- that is the point of a coroutine.
-- ------------------------------------------------------------------------------
local producer = coroutine.create(function()
    local consignments = {
        { id = 1, size = 120 },
        { id = 2, size = 45  },
        { id = 3, size = 200 },
        { id = 4, size = 60  },
        { id = 5, size = 150 },
        { id = 6, size = 30  },
        { id = 7, size = 250 },   -- deliberately large: forces the split / insufficiency path
    }

    for _, c in ipairs(consignments) do
        coroutine.yield(c.id, c.size)   -- hand control back to the scheduler with this consignment
    end

    return "All consignments produced"
end)

-- ------------------------------------------------------------------------------
-- SCHEDULER / MAIN LOOP
-- Uses coroutine.status() and the boolean returned by coroutine.resume() to
-- tell a normal yield apart from an error or the coroutine finishing (going
-- dead), and never resumes a dead coroutine.
-- ------------------------------------------------------------------------------
print("=== Starting production run ===")

while coroutine.status(producer) ~= "dead" do
    local ok, a, b = coroutine.resume(producer)

    if not ok then
        -- coroutine.resume() returned false: an error occurred inside the coroutine.
        print("  [ERROR] Producer coroutine failed: " .. tostring(a))
        break
    end

    if coroutine.status(producer) == "dead" then
        -- This resume caused the function to RETURN (not yield) -- it is finished,
        -- so 'a' here is the return value ("All consignments produced"), not a consignment.
        print("=== " .. tostring(a) .. " ===")
    else
        -- This resume caused a yield -- 'a' and 'b' are the consignment id and size.
        distributeConsignment(a, b)
    end
end

-- coroutine is now dead; resuming it again would be an error, so we stop here.
print("\nProducer coroutine status: " .. coroutine.status(producer) .. " (will not be resumed again)")

-- ------------------------------------------------------------------------------
-- Final report
-- ------------------------------------------------------------------------------
print("\n=== FINAL MARKET STORAGE ===")
for _, m in ipairs(markets) do
    print(string.format("  %-15s remaining capacity: %d bags", m.name, m.capacity))
end
