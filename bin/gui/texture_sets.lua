
local mat_list = iup.list{visiblecolumns = 12, visiblelines = 15, expand = "VERTICAL"}
local set_list = iup.list{visiblecolumns = 3, visiblelines = 15, expand = "VERTICAL"}
local diffuse_name, normal_name
local lay, materials, pos_to_set, cur_set, cur_mat_num

local tonumber = tonumber

local function Edited()
	if not cur_set then return end
	cur_set.diffuse = diffuse_name.value
	cur_set.normal = normal_name.value
end

diffuse_name = iup.text{visiblecolumns = 16, valuechanged_cb = Edited}
normal_name = iup.text{visiblecolumns = 16, valuechanged_cb = Edited}

local function clear_fields()
	diffuse_name.value = ""
	normal_name.value = ""
end

function mat_list:action(str, pos, state)
	if state == 1 and lay then
		clear_fields()
		local i = 1
		pos_to_set = {}

		set_list[1] = nil
		set_list.autoredraw = "NO"
		for _, entry in ipairs(lay) do
			local set, mat = entry.material_identifier:match("S(%d+)_M(%d+)")
			if set and mat then
				mat = tonumber(mat)
				if pos == mat then
					set_list[i] = set
					pos_to_set[i] = entry
					i = i + 1
				end
			end
		end
		set_list.autoredraw = "YES"

		cur_mat_num = pos
	end
end

function set_list:action(str, pos, state)
	if state == 1 and pos_to_set then
		local entry = pos_to_set[pos]
		if not entry then return end
		cur_set = entry
		diffuse_name.value = entry.diffuse:lower()
		normal_name.value = entry.normal:lower()
	end
end

local grid = iup.gridbox{
	iup.label{title = "Diffuse Map"}, diffuse_name,
	iup.label{title = "Normal Map"}, normal_name,
	numdiv = 2, orientation = "HORIZONTAL", homogeneouslin = "YES",
	gapcol = 10, gaplin = 8, alignmentlin = "ACENTER", sizelin = 1, nmargin = "15x0"
}

local function read(data, name, isweapon)
	if isweapon then return end
	lay = data.read.lay
	materials = data.read.mod.materials

	mat_list[1] = nil
	mat_list.autoredraw = "NO"
	for i, mat in ipairs(materials) do
		mat_list[i] = mat.name
	end
	mat_list.autoredraw = "YES"
end

local function clear()
	lay = nil
	materials = nil
	mat_list[1] = nil
	set_list[1] = nil
	cur_set = nil
	cur_mat_num = nil
	clear_fields()
end

return {
	name = "Texture Sets",
	display = iup.hbox{
		iup.frame{title = "Materials", iup.vbox{mat_list; nmargin = "5x5"}},
		iup.frame{title = "Sets", iup.vbox{set_list; nmargin = "5x5"}};
		grid,
		nmargin = "10x10"},
	read = read,
	clear = clear
}
