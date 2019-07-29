local rom = gru.n64rom_load('rom/fp.z64')
rom:crc_update()
rom:save_file('rom/fp.z64')