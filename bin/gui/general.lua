
local fields = {
	name = iup.text{visiblecolumns = 10, readonly = "YES"},
	weapon = iup.text{visiblecolumns = 10, readonly = "YES"},
	mat_count = iup.text{visiblecolumns = 10, readonly = "YES"},
	vert_count = iup.text{visiblecolumns = 10, readonly = "YES"},
	tri_count = iup.text{visiblecolumns = 10, readonly = "YES"},
	bone_count = iup.text{visiblecolumns = 10, readonly = "YES"},
	pts_count = iup.text{visiblecolumns = 10, readonly = "YES"},
	prt_count = iup.text{visiblecolumns = 10, readonly = "YES"},
	anim_count = iup.text{visiblecolumns = 10, readonly = "YES"},
}

local grid = iup.gridbox{
	iup.label{title = "Name"}, fields.name,
	iup.label{title = "Weapon"}, fields.weapon,
	iup.label{title = "Materials"}, fields.mat_count,
	iup.label{title = "Vertices"}, fields.vert_count,
	iup.label{title = "Triangles"}, fields.tri_count,
	iup.label{title = "Bones"}, fields.bone_count,
	iup.label{title = "Animations"}, fields.anim_count,
	iup.label{title = "Particle Points"}, fields.pts_count,
	iup.label{title = "Particle Emitters"}, fields.prt_count,
	numdiv = 2, orientation = "HORIZONTAL", homogeneouslin = "YES",
	gapcol = 10, gaplin = 8, alignmentlin = "ACENTER", sizelin = 8
}

local function read(data, name, isweapon)
	local r = data.read
	local model = r.mod
	local pts = r.pts
	local prt = r.prt
	fields.name.value = name or ""
	fields.weapon.value = isweapon and "Yes" or "No"
	fields.mat_count.value = #model.materials
	fields.vert_count.value = model.vertices.binary and model.vertices.count or #model.vertices
	fields.tri_count.value = model.triangles.binary and model.triangles.count or #model.triangles
	fields.bone_count.value = #model.bones
	fields.anim_count.value = r.anim_count
	fields.pts_count.value = pts and #pts or 0
	fields.prt_count.value = prt and #prt or 0
end

local function clear()
	for _, field in pairs(fields) do
		field.value = ""
	end
end

return {
	name = "Info",
	display = iup.vbox{grid; nmargin = "15x15"},
	read = read,
	clear = clear,
}
