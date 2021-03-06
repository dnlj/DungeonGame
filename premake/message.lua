msg = {}

msg.Color = {
	_isColor = true
}

function msg.Color.new(c)
	return setmetatable({
		code = c,
	},{
		__index = msg.Color,
	})
end

msg.Black = msg.Color.new(term.black)
msg.Blue = msg.Color.new(term.blue)
msg.Green = msg.Color.new(term.green)
msg.Cyan = msg.Color.new(term.cyan)
msg.Red = msg.Color.new(term.red)
msg.Purple = msg.Color.new(term.purple)
msg.Brown = msg.Color.new(term.brown)
msg.LightGray = msg.Color.new(term.lightGray)
msg.Gray = msg.Color.new(term.gray)
msg.LightBlue = msg.Color.new(term.lightBlue)
msg.LightGreen = msg.Color.new(term.lightGreen)
msg.LightCyan = msg.Color.new(term.lightCyan)
msg.LightRed = msg.Color.new(term.lightRed)
msg.Magenta = msg.Color.new(term.magenta)
msg.Yellow = msg.Color.new(term.yellow)
msg.White = msg.Color.new(term.white)

msg.Info = msg.Blue
msg.Success = msg.Green
msg.Warn = msg.Yellow
msg.Error = msg.Red

function msg.print(...)
	local orig = term.getTextColor()
	for _, v in pairs({...}) do
		if type(v) == "table" and v._isColor then
			term.setTextColor(v.code)
		else
			io.write(tostring(v))
		end
	end
	term.setTextColor(orig)
end

function msg.success(...)
	msg.print(msg.Success, ...)
end

function msg.info(...)
	msg.print(msg.Info, ...)
end

function msg.warn(...)
	msg.print(msg.Warn, ...)
end

function msg.error(...)
	msg.print(msg.Error, ...)
end

function msg.printTable(tbl, depth, shown)
	shown = shown or {}
	depth = depth or 0
	if depth == 0 then io.write(tostring(tbl), " = ") end
	depth = depth + 1
	
	local indent0 = ("  "):rep(depth - 1) 
	local indent = ("  "):rep(depth)
	io.write("{\n")
	for k,v in pairs(tbl) do
		if type(k) == "string" then
			io.write(indent, "\"", tostring(k), "\" = ")
		else
			io.write(indent, tostring(k), " = ")
		end
		
		local t = type(v)
		if t == "table" and not shown[v] then
			shown[v] = true
			msg.printTable(v, depth, shown)
		elseif t == "string" then
			io.write("\"", tostring(v), "\"\n")
		else
			io.write(tostring(v), "\n")
		end
	end
	io.write(indent0, "}\n")
end
