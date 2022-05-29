local args = { ... }
local in_rom
local opt_out_rom

while(args[1]) do
    if args[1] == "-o" then
        opt_out_rom = args[2]
        table.remove(args, 1)
        table.remove(args, 1)
    else
        in_rom = args[1]
        table.remove(args, 1)
    end
end

local make = loadfile("lua/make.lua")
local patched_rom, rom_id = make(in_rom)

if(opt_out_rom ~= nil) then
    print("Saving " .. opt_out_rom)
    patched_rom:save(opt_out_rom)
else
    print("Saving fp-" .. rom_id:lower() .. ".z64")
    patched_rom:save("fp-" .. rom_id:lower() .. ".z64")
end
