math.randomseed(os.time())

id = addInstance("sponza")
placeInstance(id, 10, 0, -10)
scaleInstance(id, 0.01, 0.01, 0.01)

pointLightArray = {}
idPoint = 1
for i = 0,1200 do
	local r = math.random()
	local g = math.random()
	local b = math.random()
	local a = 1
	
	local x = -3 + math.random() * 30
	local y = 0.5 + math.random() * 15
	local z = 3 - math.random() * 30
	local w = 1
	
	pointLightArray[idPoint] = addPointLight(r, g, b, a, x, y, z, w)
	idPoint = idPoint + 1
end

totalTime = 0
function frameUpdate(dt)
	totalTime = totalTime + dt
	local val = math.sin(totalTime) + 2
end