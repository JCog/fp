require("lua/rom_info")

local arg = { ... }

local rom = gru.n64rom_load(arg[1])
local rom_info = roms[rom:crc32()]

-- We don't have an identiable rom here, stop patching
if rom_info == nil then
    error("invalid rom", 0)
    return 1
end

local fp_version = "fp-" .. rom_info.rom_id
print("Building " .. fp_version)

print("make " .. 
fp_version ..
" build/patch/" .. fp_version .. "/hooks.gsc")
local _,_,res = os.execute("make " .. 
                           fp_version ..
                           " build/patch/" .. fp_version .. "/hooks.gsc")
if res ~= 0 then
    error("could not build fp", 0)
end

local hooks = gru.gsc_load("build/patch/" .. fp_version .. "/hooks.gsc")

print("Applying hooks")
hooks:shift(-rom_info.rom_to_ram)
hooks:apply_be(rom)

local old_ldr = rom:copy(rom_info.ldr_rom, 0x54)
local fp = gru.blob_load("build/bin/" .. fp_version .. "/fp.bin")
local payload_rom = 0x2800000
local fp_rom = payload_rom + 0x60

print("Building Loader")

local make_ldr = string.format("make -B CPPFLAGS=' -DDMA_COPY=0x%08x -DEND=0x%08x' LDFLAGS=' -Wl,--defsym,start=0x%08x'" ..
                                " ldr-" .. fp_version, rom_info.dma_func, fp:size() + fp_rom, rom_info.ldr_addr)

print(make_ldr)
local _,_,res = os.execute(make_ldr)
if(res ~= 0) then
    error("Could not build loader", 0)
end

local ldr = gru.blob_load("build/bin/ldr-" .. fp_version .. "/ldr.bin")

print("Inserting payload")
rom:write(rom_info.ldr_rom, ldr)
rom:write(payload_rom, old_ldr)
local payload = gru.blob_load("build/bin/" .. fp_version .. "/fp.bin")
rom:write(fp_rom, fp)
rom:crc_update()

return rom, rom_info.rom_id
