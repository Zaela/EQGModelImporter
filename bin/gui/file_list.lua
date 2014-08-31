
local eqg = require "luaeqg"
--local ply = require "gui/ply"
--local obj = require "gui/obj"

local list = iup.list{visiblelines = 10, expand = "VERTICAL", visiblecolumns = 10,
	sort = "YES"}

local filter = iup.text{visiblecolumns = 9, value = ""}
file_filter = filter

local FilterFileList, cur_selection, cur_name

function filter:valuechanged_cb()
	local dir = open_dir
	if dir then
		FilterFileList(dir)
	end
end

local ipairs = ipairs
local pairs = pairs
local pcall = pcall

function UpdateFileList(path, silent)
	cur_selection = nil
	cur_name = nil
	eqg.CloseDirectory(open_dir)
	open_path = path
	local s, dir = pcall(eqg.LoadDirectory, path)
	if not s then
		error_popup(dir)
		return
	end
	open_dir = dir
	by_name = {}
	if not silent then
		FilterFileList(dir)
	end
end

function FilterFileList(dir)
	list[1] = nil
	list.autoredraw = "NO"
	local f = filter.value
	local n = 1
	if f:len() > 0 then
		--make dot only match dot, not any and all characters
		--f = f:gsub("%.", "%%%.")
		--make sure last char isn't a dangling % (will throw incomplete pattern error)
		if f:find("%%", -1) then
			f = f .. "%"
		end
		for i, ent in ipairs(dir) do
			local name = ent.name
			by_name[name] = ent
			if (name:find("%.mod$") or name:find("%.mds$")) and name:find(f) then
				list[n] = name:match("[^%.]+")
				n = n + 1
			end
		end
	else
		for i, ent in ipairs(dir) do
			local name = ent.name
			by_name[name] = ent
			if name:find("%.mod$") or name:find("%.mds$") then
				list[n] = name:match("[^%.]+")
				n = n + 1
			end
		end
	end
	list.autoredraw = "YES"
end

local function GatherFiles(str)
	--find all files related to str: str.mod, str.pts, str.prt, anim_ba_1_str.ani, str_animlist.anl, etc
	local dir = open_dir
	if not dir then return end

	local src = {}
	local read = {}
	local src_anims = {}
	local read_anims = {}
	local anim_count = 0

	for i, ent in ipairs(dir) do
		local name = ent.name
		if name:find(str) then
			local ext = name:match("%.(%w+)")
			if ext then
				local s, err
				if ext == "mod" or ext == "mds" or ext == "pts" or ext == "prt" or ext == "lay" or ext == "anl" then
					s, err = pcall(eqg.OpenEntry, ent)
					if s then
						src[ext] = ent
						if ext ~= "anl" then
							s, err = pcall(_G[ext].Read, ent)
							if s then
								read[(ext == "mds") and "mod" or ext] = err
							end
						end
					end
					if not s then
						error_popup(err)
					end
				elseif ext == "ani" then
					s, err = pcall(eqg.OpenEntry, ent)
					if s then
						src_anims[name] = ent
						s, err = pcall(ani.Read, ent)
						if s then
							read_anims[name] = err
							read_anims.strict = err.strict
							anim_count = anim_count + 1
						end
					end
					if not s then
						error_popup(err)
					end
				end
			end
		end
	end

	read.anim_count = anim_count
	src.anims = src_anims
	read.anims = read_anims
	return {src = src, read = read}
end

local function GetTextures(materials)
	local dir = open_dir
	local textures = {}
	local info = {}
	local cur_info
	local function add_info(i, name, val)
		if cur_info then
			cur_info[name] = val
		else
			cur_info = {[name] = val}
			info[i] = cur_info
		end
	end
	--find and decompress all diffuse textures in the materials
	--switch this to texture sets after handling .lay files
	for i, mat in ipairs(materials) do
		for j, prop in ipairs(mat) do
			cur_info = nil
			local pname = prop.name
			if pname == "e_TextureDiffuse0" then
				local name = prop.value:lower()
				for k, ent in ipairs(dir) do
					if ent.name == name then
						local s, err = pcall(eqg.OpenEntry, ent)
						if s then
							ent.png_name = name:match("[^%.]+") .. ".png"
							ent.isDDS = (name:sub(-3) == "dds")
							textures[i] = ent
						end
						break
					end
				end
			--[[elseif pname == "e_fShininess0" then
				add_info(i, "shininess", prop.value)]]
			end
		end
		if mat.shader:find("Alpha") or mat.shader:find("Chroma") then
			add_info(i, "alpha", true)
		end
	end
	return textures, info
end

local function ViewModel(data, name)
	if not viewer.running or not open_dir then return end
	local r = data.read
	local mod = r.mod
	if mod and r.anims then
		local textures, info = GetTextures(mod.materials)
		viewer.LoadModel(mod.vertices, mod.triangles, mod.bones, mod.bone_assignments,
			r.anims, textures, info, (name:find("^it%d+")))
	end
end

function list:action(str, pos, state)
	if state == 1 then
		local data = GatherFiles(str)
		if data then
			ClearDisplays()
			UpdateDisplays(data, str, (str:match("^it%d+")))
			ViewModel(data, str)
			cur_selection = data
			cur_name = str
		end
	end
end

function StartViewer()
	viewer.Open()
	if cur_selection and cur_name then
		ViewModel(cur_selection, cur_name)
	end
end

--[[
function SaveDirEntry(entry, name)
	local dir = open_dir
	if not dir or not open_path then return end
	for i, ent in ipairs(dir) do
		if ent.name == name then
			dir[i] = entry
			s, err = pcall(eqg.WriteDirectory, open_path, dir)
			if not s then
				error_popup(err)
			end
			UpdateFileList(open_path, true)
			return
		end
	end
end
]]

--[[
function Export()
	local dlg = iup.filedlg{title = "Export to...", dialogtype = "DIR"}
	iup.Popup(dlg)
	if dlg.status == "0" then
		local path = dlg.value
		local val = list.value
		if path and val then
			local str = list[list.value]
			local outpath = path .."\\".. str .. ".ply"
			local data = by_name[str].mod
			local s, err = pcall(eqg.OpenEntry, data)
			if s then
				s, err = pcall(mod.Read, data)
				if s then
					ply.Export(err, outpath)
					local msg = iup.messagedlg{title = "Export Status", value = "Export to ".. outpath .." complete."}
					iup.Popup(msg)
					iup.Destroy(msg)
					iup.Destroy(dlg)
					return
				end
			end
			error_popup(err)
		end
	end
	iup.Destroy(dlg)
end

function Import(filter, import_func)
	local dlg = iup.filedlg{title = "Select file to import", dialogtype = "FILE",
		extfilter = filter}
	iup.Popup(dlg)
	if dlg.status == "0" then
		local path = dlg.value
		local dir = open_dir
		if path and dir then
			local id = 1000
			local input = iup.text{visiblecolumns = 12, mask = iup.MASK_UINT}
			local getid
			local but = iup.button{title = "Done", action = function() id = tonumber(input.value) or 1000 getid:hide() end}
			getid = iup.dialog{iup.vbox{
				iup.label{title = "Please enter an ID number for the incoming weapon model:"},
				input, but, gap = 12, nmargin = "15x15", alignment = "ACENTER"};
				k_any = function(self, key) if key == iup.K_CR then but:action() end end}
			iup.Popup(getid)
			iup.Destroy(getid)
			local name = "it".. id
			local pos = by_name[name]
			if pos then
				pos = pos.mod.pos
				local warn = iup.messagedlg{title = "Overwrite?",
					value = "A model with ID ".. id .." already exists in this archive. Overwrite it?",
					buttons = "YESNO", dialogtype = "WARNING"}
				iup.Popup(warn)
				local yes = (warn.buttonresponse == "1")
				iup.Destroy(warn)
				if not yes then return end
			else
				pos = #dir + 1
			end
			local data = import_func(path, dir, (pos > #dir))
			name = name .. ".mod"
			local s, err = pcall(mod.Write, data, name, eqg.CalcCRC(name))
			if s then
				dir[pos] = err
				s, err = pcall(eqg.WriteDirectory, open_path, dir)
				if s then
					local msg = iup.messagedlg{title = "Import Status", value = "Import of ".. name .." complete."}
					iup.Popup(msg)
					UpdateFileList(open_path)
					iup.Destroy(msg)
					iup.Destroy(dlg)
					return
				end
			end
			error_popup(err)
		end
	end
	iup.Destroy(dlg)
end

local function ImportPLY()
	Import("Stanford PLY (*.ply)|*.ply|", ply.Import)
end

local function ImportOBJ()
	Import("Wavefront OBJ (*.obj)|*.obj|", obj.Import)
end
]]
function list:button_cb(button, pressed, x, y)
	if button == iup.BUTTON3 and pressed == 0 then
		local has = cur_selection and "YES" or "NO"
		local mx, my = iup.GetGlobal("CURSORPOS"):match("(%d+)x(%d+)")
		local menu = iup.menu{
			iup.submenu{title = "Export Model", active = has,
				iup.menu{
					iup.item{title = "To .ply", action = Export},
				}
			},
			iup.submenu{title = "Import Model",
				iup.menu{
					iup.item{title = "From .obj", action = ImportOBJ},
					iup.item{title = "From .ply", action = ImportPLY},
				}
			}
		}
		iup.Popup(menu, mx, my)
		iup.Destroy(menu)
	end
end

return iup.vbox{iup.hbox{iup.label{title = "Filter"}, filter; alignment = "ACENTER", gap = 5}, list;
	alignment = "ACENTER", gap = 5}
