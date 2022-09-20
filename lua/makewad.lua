function usage()
    io.stderr:write("usage: makewad [<gzinject-arg>...]"
                    .. " [-m <input-rom>] [-o <output-wad>] <input-wad>\n")
    os.exit(1)
  end
  
  -- parse arguments
  local arg = {...}
  local opt_id
  local opt_title
  local opt_keyfile = "common-key.bin"
  local opt_region
  local opt_directory = "wadextract"
  local opt_rom
  local opt_out
  local opt_wad
  while arg[1] do
    if arg[1] == "-i" or arg[1] == "--channelid" then
      opt_id = arg[2]
      if opt_id == nil then usage() end
      table.remove(arg, 1)
      table.remove(arg, 1)
    elseif arg[1] == "-t" or arg[1] == "--channeltitle" then
      opt_title = arg[2]
      if opt_title == nil then usage() end
      table.remove(arg, 1)
      table.remove(arg, 1)
    elseif arg[1] == "-k" or arg[1] == "--key" then
      opt_keyfile = arg[2]
      if opt_keyfile == nil then usage() end
      table.remove(arg, 1)
      table.remove(arg, 1)
    elseif arg[1] == "-r" or arg[1] == "--region" then
      opt_region = arg[2]
      if opt_region == nil then usage() end
      table.remove(arg, 1)
      table.remove(arg, 1)
    elseif arg[1] == "-d" or arg[1] == "--directory" then
      opt_directory = arg[2]
      if opt_directory == nil then usage() end
      table.remove(arg, 1)
      table.remove(arg, 1)
    elseif arg[1] == "-m" then
      opt_rom = arg[2]
      if opt_rom == nil then usage() end
      table.remove(arg, 1)
      table.remove(arg, 1)
    elseif arg[1] == "-o" then
      opt_out = arg[2]
      if opt_out == nil then usage() end
      table.remove(arg, 1)
      table.remove(arg, 1)
    elseif opt_wad ~= nil then usage()
    else
      opt_wad = arg[1]
      table.remove(arg, 1)
    end
  end
  if opt_wad == nil then usage() end
  if opt_rom == nil then opt_rom = opt_directory .. "/content5/rom" end
  
  local gzinject = os.getenv("GZINJECT")
  if gzinject == nil or gzinject == "" then gzinject = "gzinject" end

  local _,_,romc_result = os.execute("make romc")
  if romc_result ~= 0 then return romc_result end
  
  require("lua/rom_info")
  local make = loadfile("lua/make.lua")
  
  -- extract wad
  gru.os_rm(opt_directory)
  local gzinject_cmd = gzinject ..
                       " -a extract" ..
                       " -k \"" .. opt_keyfile .. "\"" ..
                       " -d \"" .. opt_directory .. "\"" ..
                       " --verbose" ..
                       " -w \"" .. opt_wad .. "\""
  local _,_,gzinject_result = os.execute(gzinject_cmd)
  if gzinject_result ~= 0 then return gzinject_result end
  
  local romc_cmd = "./romc d " .. opt_directory .. "/content5/romc " ..
                   opt_directory .. "/content5/rom"
  local _,_,romc_result = os.execute(romc_cmd)
  if romc_result ~= 0 then return romc_result end
  
  -- make rom
  local patched_rom, rom_id = make(opt_rom)
  if rom_id == nil then
    io.stderr:write("makewad: unrecognized rom: " .. opt_rom .. "\n")
    return 2
  end
  patched_rom:save_file(opt_directory .. "/content5/rom")
  
  local romc_cmd = "./romc e " .. opt_directory .. "/content5/rom " ..
                   opt_directory .. "/content5/romc"
  
  local _,_,romc_result = os.execute(romc_cmd)
  if romc_result ~= 0 then return romc_result end
  
  os.remove(opt_directory .. "/content5/rom")
  
  -- build gzinject pack command string
  local gzinject_cmd = gzinject ..
                       " -a pack" ..
                       " -k \"" .. opt_keyfile .. "\"" ..
                       " -d \"" .. opt_directory .. "\"" ..
                       " -p \"gzi/mem_patch.gzi\"" ..
                       " --verbose" ..
                       " --cleanup"
  if opt_id ~= nil then
    gzinject_cmd = gzinject_cmd .. " -i \"" .. opt_id .. "\""
  else
    gzinject_cmd = gzinject_cmd .. " -i FP" .. rom_id
  end
  if opt_title ~= nil then
    gzinject_cmd = gzinject_cmd .. " -t \"" .. opt_title .. "\""
  else
    gzinject_cmd = gzinject_cmd .. " -t fp-" .. rom_id
  end
  if opt_region ~= nil then
    gzinject_cmd = gzinject_cmd .. " -r \"" .. opt_region .. "\""
  else
    gzinject_cmd = gzinject_cmd .. " -r 3"
  end
  if opt_out ~= nil then
    gzinject_cmd = gzinject_cmd .. " -w \"" .. opt_out .. "\""
  elseif opt_title ~= nil then
    gzinject_cmd = gzinject_cmd .. " -w \"" .. opt_title .. ".wad\""
  else
    gzinject_cmd = gzinject_cmd .. " -w \"fp-" .. rom_id .. ".wad\""
  end
  -- execute
  local _,_,gzinject_result = os.execute(gzinject_cmd)
  if gzinject_result ~= 0 then return gzinject_result end
  
  return 0
