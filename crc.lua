local args = {...}
local romu = gru.n64rom_load(args[1])
romu:crc_update()
romu:save_file(args[1])