math.randomseed(os.time())


function frameUpdate(dt)

end

pointLightArray = {}
idx = 1
for i = 0,1000 do
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

instanceArray = {}
idx = 1
for i = 0,20 do
	for j = 0,20 do
		instanceArray[idx] = addInstance("suzanne")
		
		local x = 2 * i
		local y = 0
		local z = -2 * j
		placeInstance(instanceArray[idx], x, y, z)
	end
end

id = addInstance("floor")
placeInstance(id, 0, -1, 0)

id = addInstance("chalet")
placeInstance(id, 20, 0, -20)