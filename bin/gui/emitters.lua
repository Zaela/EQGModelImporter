
local emitter_list = iup.list{visiblecolumns = 3, visiblelines = 15, expand = "VERTICAL"}
local points, emitters, cur_emitter, cur_emitter_pos, pos_to_emitter, cur_id, cur_data, cur_point_name
local emitter_fields

local tonumber = tonumber

local function Edited()
	if cur_emitter then
		cur_emitter.particle_id = tonumber(emitter_fields.id.value)
		cur_emitter.duration = tonumber(emitter_fields.duration.value)
		for i = 1, 8 do
			cur_emitter.unknown[i] = tonumber(emitter_fields.unknown[i].value)
		end
	end
end

emitter_fields = {
	id = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT, valuechanged_cb = Edited},
	duration = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT, valuechanged_cb = Edited},
	unknown = {},
}

local emitter_grid = iup.gridbox{
	iup.label{title = "Particle ID"}, emitter_fields.id,
	iup.label{title = "Duration?"}, emitter_fields.duration,
	numdiv = 2, orientation = "HORIZONTAL", homogeneouslin = "YES",
	gapcol = 10, gaplin = 8, alignmentlin = "ACENTER", sizelin = 0
}

for i = 1, 8 do
	local t
	if i == 7 then
		t = iup.text{visiblecolumns = 12, valuechanged_cb = Edited}
	else
		t = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT, valuechanged_cb = Edited}
	end
	emitter_fields.unknown[i] = t
	iup.Append(emitter_grid, iup.label{title = "Unknown".. i})
	iup.Append(emitter_grid, t)
end

function emitter_list:action(str, pos, state)
	if state == 1 and emitters then
		local e = pos_to_emitter[pos]
		if not e then return end
		cur_emitter = e
		cur_emitter_pos = pos
		emitter_fields.id.value = e.particle_id
		emitter_fields.duration.value = e.duration
		local u = e.unknown
		for i, v in ipairs(u) do
			if i == 7 then
				emitter_fields.unknown[7].value = string.format("0x%0.8X", v)
			else
				emitter_fields.unknown[i].value = v
			end
		end
	end
end

local function clear_fields()
	emitter_fields.id.value = ""
	emitter_fields.duration.value = ""
	for _, field in pairs(emitter_fields.unknown) do
		field.value = ""
	end
end

local emit = {}

function emit.update(id, name)
	cur_id = id
	if name then cur_point_name = name end
	if not emitters then return end
	clear_fields()
	emitter_list[1] = nil
	emitter_list.autoredraw = "NO"
	pos_to_emitter = {}
	local i = 1
	for _, e in ipairs(emitters) do
		if e.internal_id == id then
			pos_to_emitter[i] = e
			emitter_list[i] = i
			i = i + 1
		end
	end
	emitter_list.autoredraw = "YES"
end

function emit.set_ids(name_ids)
	if not emitters then return end
	for _, e in ipairs(emitters) do
		local id = name_ids[e.particle_name]
		e.internal_id = id or 0
	end
end

function emit.remove(id)
	if not emitters then return end
	local to_remove = {}
	for i, e in ipairs(emitters) do
		if e.internal_id == id then
			table.insert(to_remove, i)
		end
	end

	local n = 0
	for _, i in ipairs(to_remove) do
		table.remove(emitters, i - n)
		n = n + 1
	end

	if cur_emitter.internal_id == id then
		cur_emitter = nil
	end

	if cur_id == id then
		emit.clear()
	end
end

function emit.name_change(id, to)
	cur_point_name = to
	if not emitters then return end
	for _, e in ipairs(emitters) do
		if e.internal_id == id then
			e.particle_name = to
		end
	end
end

function emit.read(data)
	cur_data = data
	emitters = data.read.prt
	points = data.read.pts

	emitter_list[1] = nil
	cur_emitter = nil
end

function emit.clear()
	clear_fields()
	emitter_list[1] = nil
	emitters = nil
	points = nil
	cur_emitter = nil
	cur_id = nil
	cur_data = nil
	cur_point_name = nil
end

function emit.soft_clear()
	clear_fields()
	emitter_list[1] = nil
	cur_id = nil
	cur_emitter = nil
end

function emit.copy(from_id, to_id, name)
	if not emitters then return end
	for _, e in ipairs(emitters) do
		if e.internal_id == from_id then
			local add = {}
			copy_tbl_recurse(e, add)
			add.internal_id = to_id
			add.particle_name = name
			table.insert(emitters, add)
		end
	end
end

local function AddEmitter()
	if not cur_data then return end
	if not emitters then
		emitters = {}
		cur_data.read.prt = emitters
	end

	table.insert(emitters, {
		internal_id = cur_id,
		particle_name = cur_point_name,
		particle_id = 1,
		duration = 5000,
		unknown = {
			0, 0, 0, 0, 0, 0, 0xFFFFFFFF, 0
		},
	})

	emit.update(cur_id)
end

local function CopyEmitter()
	if not cur_emitter then return end

	local new = {}
	copy_tbl_recurse(cur_emitter, new)

	table.insert(emitters, new)
	emit.update(cur_id)
end

local function RemoveEmitter()
	if not emitters or not cur_emitter then return end
	for i, e in ipairs(emitters) do
		if e == cur_emitter then
			table.remove(emitters, i)
			break
		end
	end
	emit.update(cur_id)
end

function emitter_list:button_cb(button, pressed, x, y)
	if cur_data and button == iup.BUTTON3 and pressed == 0 then
		local mx, my = iup.GetGlobal("CURSORPOS"):match("(%d+)x(%d+)")
		local menu = iup.menu{
			iup.item{title = "Add Emitter", action = AddEmitter},
			iup.item{title = "Copy Emitter", action = CopyEmitter, active = cur_emitter and "YES" or "NO"},
			iup.separator{},
			iup.item{title = "Remove Emitter", action = RemoveEmitter, active = cur_emitter and "YES" or "NO"},
		}
		iup.Popup(menu, mx, my)
		iup.Destroy(menu)
	end
end

emit.display = iup.hbox{iup.frame{title = "Emitters",
	iup.hbox{iup.vbox{emitter_list; nmargin = "5x5"}, iup.vbox{emitter_grid; nmargin = "5x5"}}};
	nmargin = "10x10"}

return emit
