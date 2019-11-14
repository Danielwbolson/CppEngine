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

function keyInput(keys)
	if keys.one then
		-- I have beautifully coded the below to not work at runtime...
		--local index = addInstance("sponza")
		--placeInstance(index, 20, 0, -10)
		--scaleInstance(index, 0.01, 0.01, 0.01)
		changeColor(id, 1, 0, 0)
	elseif keys.two then
		changeColor(id, 0, 1, 0)
	elseif keys.three then
		changeColor(id, 0, 0, 1)
	elseif keys.four then
	-- THIS ONE DOES NOT WORK AS EXPECTED
	-- STILL LIT THO
		disableTextures(id)
	elseif keys.five then
		enableTextures(id)
	elseif keys.six then
		changeColor(id, 1, 1, 1)
	end
	
end