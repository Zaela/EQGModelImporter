
local lfs = require "lfs"

function copy_tbl_recurse(from, to)
	for k, v in pairs(from) do
		if type(v) == "table" then
			local add = {}
			to[k] = add
			copy_tbl_recurse(v, add)
		else
			to[k] = v
		end
	end
end

local displays = {
	require "gui/general",
	require "gui/material",
	require "gui/bones",
	require "gui/animations",
	require "gui/points",
	require "gui/texture_sets",
}

local tabs = iup.tabs{padding = "5x5"}

for i = 1, #displays do
	local d = displays[i]
	tabs["tabtitle".. (i - 1)] = d.name
	iup.Append(tabs, d.display)
end

--[[
function tabs:tabchangepos_cb(pos)
	local d = displays[pos + 1]
	if d.load then
		d.load()
	end
end
]]

function ClearDisplays()
	for i, d in ipairs(displays) do
		d.clear()
	end
end

function UpdateDisplays(data, name, isweapon)
	for i, d in ipairs(displays) do
		d.read(data, name, isweapon)
	end
end

return tabs
