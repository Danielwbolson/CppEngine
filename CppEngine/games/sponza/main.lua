--math.randomseed(os.time())
math.randomseed(0)

id = addInstance("sponza")
placeInstance(id, 10, 0, -10)
scaleInstance(id, 0.01, 0.01, 0.01)


--sun = addDirectionalLight(0.945, 0.9, 0.8, -0.5, -1.2, -0.7)
sun = addDirectionalLight(.945 * 1.5, .9 * 1.5, .8 * 1.5, 0, -1, 0)


pointLightArray = {}
idPoint = 1
for i = 0,0 do
	local r = 0.945
	local g = 0.9
	local b = 0.8
	
	local x = -5 + math.random() * 32
	local y = 0 + math.random() * 12
	local z = -1 - math.random() * 18
	
	pointLightArray[idPoint] = addPointLight(r, g, b, x, y, z)
	idPoint = idPoint + 1
end

totalTime = 0
angle = 0;
function frameUpdate(dt)
	totalTime = totalTime + dt
	local val = math.sin(totalTime) + 2
end

function keyInput(keys)
	if keys.one then
		rotateSunX(sun, -0.004);		
	elseif keys.two then
		rotateSunX(sun, 0.004);	
	end
	
end