math.randomseed(os.time())


function frameUpdate(dt)

end

pointLightArray = {}
idx = 1
for i = 0,50 do
	local r = 1
	local g = 1
	local b = 1
	local a = 1
	
	local x = 10 + math.random() * 20
	local y = 0.5
	local z = -5 + math.random() * 10
	local w = 1
	
	pointLightArray[idx] = addPointLight(r, g, b, a, x, y, z, w)
	idx = idx + 1
end

-- instanceArray = {}
-- idx = 1
-- for i = 0,20 do
	-- for j = 0,20 do
		-- instanceArray[idx] = addInstance("chalet")
		
		-- local x = 2 * i
		-- local y = 0
		-- local z = -2 * j
		-- placeInstance(instanceArray[idx], x, y, z)
	-- end
-- end

id = addInstance("floor")
placeInstance(id, 0, -1, 0)

id = addInstance("chalet")
placeInstance(id, 20, -0.5, 0)