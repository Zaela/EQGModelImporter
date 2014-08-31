
local point_list = iup.list{visiblecolumns = 12, visiblelines = 15, expand = "VERTICAL"}
local bones, points, cur_point, cur_point_pos, cur_data, cur_isweapon
local name_list, attach_list, point_fields
local UpdateNameList

local emit = require "gui/emitters"
local tonumber = tonumber
local type = type

local default_names = {
	DAMAGE_POINT = true,
	SPELLPOINT_DEFAULT = true,
	SPELLPOINT_CHEST = true,
	SPELLPOINT_HEAD = true,
	SPELLPOINT_LHAND = true,
	SPELLPOINT_RHAND = true,
	SPELLPOINT_LFOOT = true,
	SPELLPOINT_RFOOT = true,
	SPELLPOINT_LEYE = true,
	SPELLPOINT_REYE = true,
	SPELLPOINT_MOUTH = true,
	SPELLPOINT_EARS = true,
	SPELLPOINT_LVENT = true,
	SPELLPOINT_RVENT = true,
}

local function Edited()
	if cur_point then
		local name = name_list.value
		if name ~= cur_point.particle_name then
			emit.name_change(cur_point_pos, name)
			local old = cur_point.particle_name
			cur_point.particle_name = name
			point_list[cur_point_pos] = name
			if default_names[name] or default_names[old] then
				UpdateNameList()
				name_list.value = name
			end
		end
		local t = cur_point.translation
		t.x = tonumber(point_fields.x.value)
		t.y = tonumber(point_fields.y.value)
		t.z = tonumber(point_fields.z.value)
		t = cur_point.rotation
		t.x = tonumber(point_fields.rot_x.value)
		t.y = tonumber(point_fields.rot_y.value)
		t.z = tonumber(point_fields.rot_z.value)
		t = cur_point.scale
		t.x = tonumber(point_fields.scale_x.value)
		t.y = tonumber(point_fields.scale_y.value)
		t.z = tonumber(point_fields.scale_z.value)

		local v = tonumber(attach_list.value)
		cur_point.attach_name = (v < 2) and "ATTACH_TO_ORIGIN" or bones[v - 1].name
	end
end

function UpdateNameList()
	if not points or cur_isweapon then return end
	local avail = {}
	for k in pairs(default_names) do
		avail[k] = true
	end

	for _, p in ipairs(points) do
		local name = p.particle_name
		if avail[name] then
			avail[name] = nil
		end
	end

	local i = 1
	name_list[1] = nil
	name_list.autoredraw = "NO"
	for k in pairs(avail) do
		name_list[i] = k
		i = i + 1
	end
	name_list.autoredraw = "YES"
end

name_list = iup.list{visiblecolumns = 10, dropdown = "YES", editbox = "YES", visible_items = 10,
	valuechanged_cb = function()
		--for some dumb reason this does't get the updated values like it does for everything else when editbox
		iup.SetIdle(function()
			Edited()
			iup.SetIdle(nil)
		end)
	end}

attach_list = iup.list{visiblecolumns = 10, dropdown = "YES", visible_items = 15,
	valuechanged_cb = Edited, "ATTACH_TO_ORIGIN"}

point_fields = {
	x = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, valuechanged_cb = Edited},
	y = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, valuechanged_cb = Edited},
	z = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, valuechanged_cb = Edited},
	rot_x = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, valuechanged_cb = Edited},
	rot_y = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, valuechanged_cb = Edited},
	rot_z = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, valuechanged_cb = Edited},
	scale_x = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, valuechanged_cb = Edited},
	scale_y = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, valuechanged_cb = Edited},
	scale_z = iup.text{visiblecolumns = 12, mask = iup.MASK_FLOAT, valuechanged_cb = Edited},
}

local point_grid = iup.gridbox{
	iup.label{title = "Name"}, name_list,
	iup.label{title = "Attach To"}, attach_list,
	iup.label{title = "Translation X"}, point_fields.x,
	iup.label{title = "Translation Y"}, point_fields.y,
	iup.label{title = "Translation Z"}, point_fields.z,
	iup.label{title = "Rotation X"}, point_fields.rot_x,
	iup.label{title = "Rotation Y"}, point_fields.rot_y,
	iup.label{title = "Rotation Z"}, point_fields.rot_z,
	iup.label{title = "Scale X"}, point_fields.scale_x,
	iup.label{title = "Scale Y"}, point_fields.scale_y,
	iup.label{title = "Scale Z"}, point_fields.scale_z,
	numdiv = 2, orientation = "HORIZONTAL", homogeneouslin = "YES",
	gapcol = 10, gaplin = 8, alignmentlin = "ACENTER", sizelin = 2
}

function point_list:action(str, pos, state)
	if state == 1 and points then
		local p = points[pos]
		name_list.value = p.particle_name
		local t = p.translation
		point_fields.x.value = t.x
		point_fields.y.value = t.y
		point_fields.z.value = t.z
		t = p.rotation
		point_fields.rot_x.value = t.x
		point_fields.rot_y.value = t.y
		point_fields.rot_z.value = t.z
		t = p.scale
		point_fields.scale_x.value = t.x
		point_fields.scale_y.value = t.y
		point_fields.scale_z.value = t.z

		local name = p.attach_name
		if name == "ATTACH_TO_ORIGIN" or not bones then
			attach_list.value = 1
		else
			for i, bone in ipairs(bones) do
				if name == bone.name then
					attach_list.value = i + 1
					break
				end
			end
		end

		cur_point = p
		cur_point_pos = pos
		emit.update(pos, str)
	end
end

local function UpdatePointList()
	point_list[1] = nil
	point_list.autoredraw = "NO"
	for i, p in ipairs(points) do
		point_list[i] = p.particle_name
	end
	point_list.autoredraw = "YES"
end

local function UpdateEmitterIDs()
	local name_to_pos = {}
	for i, p in ipairs(points) do
		name_to_pos[p.particle_name] = i
	end
	emit.set_ids(name_to_pos)
end

local function read(data, name, isweapon)
	cur_data = data
	cur_isweapon = isweapon
	emit.read(data)
	points = data.read.pts
	if not points then return end
	bones = data.read.mod.bones

	UpdatePointList()
	UpdateEmitterIDs()
	UpdateNameList()

	attach_list[2] = nil
	if not bones then return end
	attach_list.autoredraw = "NO"
	local i = 2
	for _, bone in ipairs(bones) do
		attach_list[i] = bone.name
		i = i + 1
	end
	attach_list.autoredraw = "YES"
end

local function clear_fields()
	for _, field in pairs(point_fields) do
		field.value = ""
	end
	name_list.value = ""
	attach_list.value = 1
end

local function clear()
	emit.clear()
	clear_fields()
	point_list[1] = nil
	attach_list[2] = nil
	name_list[1] = nil
	bones = nil
	points = nil
	cur_point = nil
	cur_data = nil
	cur_isweapon = nil
end

local function AddPoint()
	if not cur_data then return end
	if not points then
		points = {}
		cur_data.read.pts = points
	end

	table.insert(points, {
		particle_name = "UnnamedPoint",
		attach_name = "ATTACH_TO_ORIGIN",
		translation = {x = 0, y = 0, z = 0},
		rotation = {x = 0, y = 0, z = 0},
		scale = {x = 1, y = 1, z = 1},
	})

	UpdatePointList()
end

local function CopyPoint()
	if not cur_point then return end

	local new = {}
	copy_tbl_recurse(cur_point, new)
	new.particle_name = "UnnamedCopy"

	emit.copy(cur_point_pos, #points + 1, "UnnamedCopy")

	table.insert(points, new)
	UpdatePointList()
end

local function RemovePoint()
	if not points or not cur_point then return end
	table.remove(points, cur_point_pos)
	UpdatePointList()
	UpdateEmitterIDs()
	emit.soft_clear()
	clear_fields()
	if default_names[cur_point.particle_name] then
		UpdateNameList()
	end
	cur_point = nil
end

function point_list:button_cb(button, pressed, x, y)
	if cur_data and button == iup.BUTTON3 and pressed == 0 then
		local mx, my = iup.GetGlobal("CURSORPOS"):match("(%d+)x(%d+)")
		local menu = iup.menu{
			iup.item{title = "Add Point", action = AddPoint},
			iup.item{title = "Copy Point", action = CopyPoint, active = cur_point and "YES" or "NO"},
			iup.separator{},
			iup.item{title = "Remove Point", action = RemovePoint, active = cur_point and "YES" or "NO"},
		}
		iup.Popup(menu, mx, my)
		iup.Destroy(menu)
	end
end

return {
	name = "Particles",
	display = iup.hbox{iup.frame{title = "Points",
		iup.hbox{iup.vbox{point_list; nmargin = "5x5"}, iup.vbox{point_grid; nmargin = "5x5"}}},
		emit.display;
		nmargin = "10x10"},
	read = read,
	clear = clear
}
