math.randomseed(os.time())


function frameUpdate(dt)

end

id = addInstance("floor")
placeInstance(id, 0, -1, 0)
scaleInstance(id, 2, 1, 2)


pointLightArray = {}
idx = 1
for i = 0,1000 do
	local r = math.random()
	local g = math.random()
	local b = math.random()
	local a = 1
	
	local x = -3 + math.random() * 30
	local y = 0.5
	local z = 3 - math.random() * 30
	local w = 1
	
	pointLightArray[idx] = addPointLight(r, g, b, a, x, y, z, w)
	idx = idx + 1
end


instanceArray = {}
idx = 1
for i = 0,4 do
	for j = 0,4 do
		instanceArray[idx] = addInstance("chalet")
		
		local x = 6 * i
		local y = 0
		local z = -6 * j
		placeInstance(instanceArray[idx], x, y, z)
		scaleInstance(instanceArray[idx], 3, 3, 3)
	end
end


id = addInstance("mansion")
placeInstance(id, 40, 0, -10)

mansionLightArray = {}
idx = 1
for i = 0,20 do
	local r = .6
	local g = .3
	local b = 0
	local a = 1
	
	local x = 38 + math.random() * 4
	local y = 0.5
	local z = -8 - math.random() * 4
	local w = 1
	
	mansionLightArray[idx] = addPointLight(r, g, b, a, x, y, z, w)
	idx = idx + 1
end

