math.randomseed(os.time())


id = addInstance("floor")
placeInstance(id, 0, -1, 0)
scaleInstance(id, 2, 1, 2)

pointLightArray = {}
idPoint = 1
for i = 0,1000 do
	local r = math.random() * 2
	local g = math.random() * 2
	local b = math.random() * 2
	local a = 1
	
	local x = -3 + math.random() * 30
	local y = math.random() * 5
	local z = 3 - math.random() * 30
	local w = 1
	
	pointLightArray[idPoint] = addPointLight(r, g, b, a, x, y, z, w)
	idPoint = idPoint + 1
end


cyborgArray = {}
idCyborg = 0
for i = 0,4 do
	for j = 0,4 do
		idCyborg = idCyborg + 1
			
		cyborgArray[idCyborg] = addInstance("cyborg")
		
		local x = 6 * i
		local y = 0
		local z = -6 * j
		placeInstance(cyborgArray[idCyborg], x, y, z)
		scaleInstance(cyborgArray[idCyborg], 1, 1, 1)
		
	end
end

totalTime = 0
function frameUpdate(dt)
	totalTime = totalTime + dt
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

