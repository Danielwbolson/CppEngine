math.randomseed(os.time())


function frameUpdate(dt)

end

pointLightArray = {}
idx = 1
for i = 0,1 do
	local r = math.random() * 4
	local g = math.random() * 4
	local b = math.random() * 4
	local a = 1
	
	local x = math.random() * 40
	local y = 0.5
	local z = -math.random() * 40
	local w = 1
	
	pointLightArray[idx] = addPointLight(r, g, b, a, x, y, z, w)
	idx = idx + 1
end
