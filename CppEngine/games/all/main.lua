math.randomseed(os.time())


id = addInstance("floor")
placeInstance(id, 0, -1, 0)
scaleInstance(id, 2, 1, 2)

id = addInstance("sponza")
placeInstance(id, 10, 0, -10)
scaleInstance(id, 0.01, 0.01, 0.01)

pointLightArray = {}
idPoint = 1
for i = 0,1000 do
	local r = math.random()
	local g = math.random()
	local b = math.random()
	local a = 1
	
	local x = -3 + math.random() * 30
	local y = 0.5
	local z = 3 - math.random() * 30
	local w = 1
	
	pointLightArray[idPoint] = addPointLight(r, g, b, a, x, y, z, w)
	idPoint = idPoint + 1
end


chaletArray = {}
idChalet = 0
for i = 0,4 do
	for j = 0,4 do
		idChalet = idChalet + 1
			
		chaletArray[idChalet] = addInstance("chalet")
		
		local x = 6 * i
		local y = 0
		local z = -6 * j
		placeInstance(chaletArray[idChalet], x, y, z)
		scaleInstance(chaletArray[idChalet], 3, 3, 3)
		
	end
end



idMansion = addInstance("mansion")
placeInstance(idMansion, 40, 0, -10)

mansionLightArray = {}
idMansionLight = 1
for i = 0,20 do
	local r = .6
	local g = .3
	local b = 0
	local a = 1
	
	local x = 38 + math.random() * 4
	local y = 0.5
	local z = -8 - math.random() * 4
	local w = 1
	
	mansionLightArray[idMansionLight] = addPointLight(r, g, b, a, x, y, z, w)
	idMansionLight = idMansionLight + 1
end



totalTime = 0
function frameUpdate(dt)
	totalTime = totalTime + dt
	local val = math.sin(totalTime) + 2
	
	for i = 1,idChalet do
		scaleInstance(chaletArray[i], val, val, val)
		rotateInstance(chaletArray[i], math.sin(totalTime), 0)
	end
	scaleInstance(idMansion, val, val, val)
	rotateInstance(idMansion, math.sin(totalTime), 0)
end

