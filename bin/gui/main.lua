
local lfs = require "lfs"
local eqg = require "luaeqg"

function assert(result, msg)
	if result then return result end
	local err = iup.messagedlg{buttons = "OK", dialogtype = "ERROR", title = "Error", value = msg}
	iup.Popup(err)
	iup.Close()
end

function error_popup(msg)
	local err = iup.messagedlg{buttons = "OK", dialogtype = "ERROR", title = "Error", value = msg}
	iup.Popup(err)
	iup.Destroy(err)
end

local log_file = assert(io.open("log.txt", "w+"))
log_file:setvbuf("no")
function log_write(...)
	log_file:write("[", os.date(), "] ", ...)
	log_file:write("\r\n")
end

log_write "Loading UI"

local dir_list = require "gui/dir_list"
local file_list = require "gui/file_list"
local tabs = require "gui/displays"

local title = "EQG Model Importer v0.0"
local window

function Save(silent)
	local dir = open_dir
	local path = open_path
	if not dir or not path then return false end

	--save things here

	if not silent then
		local msg = iup.messagedlg{title = "Saved", value = "Saved successfully."}
		iup.Popup(msg)
		iup.Destroy(msg)
	end
	log_write "Saved successfully"
	return true
end

local function SetSearchFolder()
	local dlg = iup.filedlg{title = "Select EQG Search Folder", dialogtype = "DIR", directory = lfs.currentdir()}
	iup.Popup(dlg)
	if dlg.status == "0" then
		local path = dlg.value
		if path then
			search_path = path
			UpdateDirList(path)
			local v = settings and settings.viewer
			log_write "Writing gui/settings.lua"
			local f = assert(io.open("gui/settings.lua", "w+"))
			f:write("\nsettings = {\n\tfolder = \"", (path:gsub("\\", "\\\\")), "\",\n")
			f:write("\tviewer = {\n\t\twidth = ", v and v.width or 500, ",\n\t\theight = ", v and v.height or 500, ",\n")
			f:write("\t}\n}\n")
			f:close()
		end
	end
end

local menu = iup.menu{
	iup.submenu{
		title = "&File";
		iup.menu{
			iup.item{title = "Set EQG Search Folder", action = SetSearchFolder},
			iup.item{title = "&Save Current Model", action = function() Save() end, active = "NO"},
			iup.separator{},
			iup.item{title = "&Quit", action = function() return iup.CLOSE end},
		},
	},
	--[[iup.submenu{
		title = "Utility";
		iup.menu{
			iup.item{title = "Convert S3D Zone", action = s3d.ConvertZone},
			iup.item{title = "Export Zone", action = obj.Export},
		},
	},]]
	iup.submenu{
		title = "Viewer";
		iup.menu{
			iup.item{title = "Start Viewer", action = StartViewer},
			iup.separator{},
			iup.item{title = "Close Viewer", action = viewer.Close},
		},
	},
}

window = assert(iup.dialog{
	iup.hbox{dir_list, file_list, iup.hbox{tabs; nmargin = "10x0"}; nmargin = "10x10"};
	title = title, menu = menu})

function window:k_any(key)
	if key == iup.K_ESC then
		return iup.CLOSE
	end
end

local function LoadSettings()
	local set = loadfile("gui/settings.lua")
	if set then
		set()
		if settings and settings.folder then
			search_path = settings.folder
			UpdateDirList(search_path)
			return
		end
	end
	SetSearchFolder()
end

window:show()

LoadSettings()
LoadSettings = nil

iup.MainLoop()

eqg.CloseDirectory(open_dir)

iup.Close()

log_write "Shutting down cleanly"
log_file:close()
