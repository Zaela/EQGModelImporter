
local list = iup.list{visiblecolumns = 12, visiblelines = 15, expand = "VERTICAL"}
local anims, cur_anim

local fields = {
	name = iup.text{visiblecolumns = 12, readonly = "YES"},
	frames = iup.text{visiblecolumns = 12, readonly = "YES"},
}

local grid = iup.gridbox{
	iup.label{title = "Name"}, fields.name,
	iup.label{title = "Frames"}, fields.frames,
	numdiv = 2, orientation = "HORIZONTAL", homogeneouslin = "YES",
	gapcol = 10, gaplin = 8, alignmentlin = "ACENTER", sizelin = 1, nmargin = "15x0"
}

function list:action(str, pos, state)
	if state == 1 and anims then
		cur_anim = anims[str]
		fields.name.value = str
		fields.frames.value = cur_anim.ROOT_BONE.count
	end
end

local function read(data)
	anims = data.read.anims
	list[1] = nil
	list.autoredraw = "NO"
	local i = 1
	for name in pairs(anims) do
		if name ~= "strict" then
			list[i] = name
			i = i + 1
		end
	end
	list.autoredraw = "YES"
end

local function clear()
	for _, field in pairs(fields) do
		field.value = ""
	end
	list[1] = nil
	anims = nil
end

return {
	name = "Animations",
	display = iup.hbox{list, grid; nmargin = "10x10"},
	read = read,
	clear = clear
}
