
local list = iup.list{visiblecolumns = 12, visiblelines = 15, expand = "VERTICAL"}

local function read(data)
	local bones = data.read.mod.bones
	list[1] = nil
	list.autoredraw = "NO"
	for i, bone in ipairs(bones) do
		local name = bone.name:upper()
		bone.name = name
		list[i] = name
	end
	list.autoredraw = "YES"
end

local function clear()
	list[1] = nil
end

return {
	name = "Skeleton",
	display = iup.hbox{list; nmargin = "10x10"},
	read = read,
	clear = clear
}
