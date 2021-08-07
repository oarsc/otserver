-- This file is part of Jiddo's advanced NpcSystem v3.0x. This npcsystem is free to use by anyone, for any purpuse. 
-- Initial release date: 2007-02-21
-- Credits: Jiddo, honux(I'm using a modified version of his Find function).
-- Please include full credits whereever you use this system, or parts of it.
-- For support, questions and updates, please consult the following thread:
-- http://opentibia.net/topic/59592-release-advanced-npc-system-v30a/

if(KeywordHandler == nil) then
	
	BEHAVIOR_SIMPLE = 1 -- Does not support nested keywords. If you choose this setting you must use a variable such as 'talkState' to keep track of how to handle keywords.
	BEHAVIOR_NORMAL = 2 -- Default behvaior. If a sub-keyword is not found, then the root is searched, not the parent hierarchy,
	BEHAVIOR_NORMAL_EXTENDED = 3 -- Same as BEHAVIOR_NORMAL but it also searches through the last node's parent.
	BEHAVIOR_COMPLEX = 4 -- Extended behavior. It a sub-keyword is not found, then the entire keyword hierarchy is searched upwards intil root is reached.
	
	
	-- BEHAVIOR_NORMAL_EXTENDED is recommended as it (probably) mimics the behavior of real Tibia's NPCs the most. 
	--		However, you are strongly recommended to test some (or all) other settings as well as it might suit you better.
	--		Also note that not much difference can be seen with the different settings unless you have a npc with a quite heavy 
	--		nestled keyword hierarchy. 
	-- Note: BEHAVIOR_SIMPLE should not be used unless you have any special reason to do so as it forces you to keep track of talkStates etc.
	--		This was pretty much the method used in the 2.0x versions of this system. It is here mainly for compability issues.
	KEYWORD_BEHAVIOR = BEHAVIOR_NORMAL_EXTENDED
	
	
	KeywordNode = {
		keywords = nil,
		callback = nil,
		parameters = nil,
		children = nil,
		parent = nil
	}
	
	-- Created a new keywordnode with the given keywords, callback function and parameters and without any childNodes.
	function KeywordNode:new(keys, func, param)
		local obj = {}
		obj.keywords = keys
		obj.callback = func
		obj.parameters = param
		obj.children = {}
		setmetatable(obj, self)
		self.__index = self
		return obj
	end
	
	-- Calls the underlying callback function if it is not nil.
	function KeywordNode:processMessage(cid, message)
		return (self.callback == nil or self.callback(cid, message, self.keywords, self.parameters, self))
	end
	
	-- Returns true if message contains all patterns/strings found in keywords.
	function KeywordNode:checkMessage(message)
		local ret = true
		if(self.keywords.callback ~= nil) then
			return self.keywords.callback(self.keywords, message)
		end
		for i,v in ipairs(self.keywords) do
			if(type(v) == 'string') then
				local a, b = string.find(message, v)
				if(a == nil or b == nil) then
					ret = false
					break
				end
			end
		end
		return ret
	end
	
	-- Returns the parent of this node or nil if no such node exists.
	function KeywordNode:getParent()
		return self.parent
	end
	
	-- Returns an array of the callback function parameters assosiated with this node.
	function KeywordNode:getParameters()
		return self.parameters
	end
	
	-- Returns an array of the triggering keywords assosiated with this node.
	function KeywordNode:getKeywords()
		return self.keywords
	end
	
	-- Adds a childNode to this node. Creates the childNode based on the parameters (k = keywords, c = callback, p = parameters)
	function KeywordNode:addChildKeyword(keywords, callback, parameters)
		local new = KeywordNode:new(keywords, callback, parameters)
		return self:addChildKeywordNode(new)
	end
	
	-- Adds a pre-created childNode to this node. Should be used for example if several nodes should have a common child.
	function KeywordNode:addChildKeywordNode(childNode)
		table.insert(self.children, childNode)
		childNode.parent = self
		return childNode
	end

	function KeywordNode:sort()
		table.sort(self.children, sorterFunction)

		for i, name in pairs(self.children) do
			name:sort()
		end
	end

	function sorterFunction(kNode1, kNode2)
		return string.len(kNode1:getKeywords()[1]) > string.len(kNode2:getKeywords()[1])
	end
	
	
	
	KeywordHandler = {
		root = nil,
	}
	
	-- Creates a new keywordhandler with an empty rootnode. 
	function KeywordHandler:new()
		local obj = {}
		obj.root = KeywordNode:new(nil, nil, nil)
		setmetatable(obj, self)
		self.__index = self
		return obj
	end
	
	-- Makes sure the correct childNode of lastNode gets a chance to process the message. 
	--	The behavior of this function depends much on the KEYWORD_BEHAVIOR.
	function KeywordHandler:processMessage(cid, message)
		local node = self.getLastNode(cid)
		if(node == nil) then
			error('No root node found.')
			return false
		end
		if(KEYWORD_BEHAVIOR == BEHAVIOR_SIMPLE) then
			local ret = self:processNodeMessage(node, cid, message)
			if(ret) then
				return true
			end
		elseif(KEYWORD_BEHAVIOR == BEHAVIOR_NORMAL or KEYWORD_BEHAVIOR == BEHAVIOR_NORMAL_EXTENDED) then
			local ret = self:processNodeMessage(node, cid, message)
			if(ret) then
				return true
			end
			if(KEYWORD_BEHAVIOR == BEHAVIOR_NORMAL_EXTENDED and node:getParent()) then
				node = node:getParent() -- Search through the parent.
				local ret = self:processNodeMessage(node, cid, message)
				if(ret) then
					return true
				end
			end
			if(node ~= self:getRoot()) then
				node = self:getRoot() -- Search through the root.
				local ret = self:processNodeMessage(node, cid, message)
				if(ret) then
					return true
				end
			end
		elseif(KEYWORD_BEHAVIOR == BEHAVIOR_COMPLEX) then
			while true do
				
				local ret = self:processNodeMessage(node, cid, message)
				if(ret) then
					return true
				end
				
				if(node:getParent() ~= nil) then
					node = node:getParent() -- Move one step upwards in the hierarchy.
				else
					break
				end
			end
		else
			error('Unknown keyword behavior.')
		end
		return false
	end
	
	-- Tries to process the given message using the node parameter's children and calls the node's callback function if found.
	--	Returns the childNode which processed the message or nil if no such node was found.
	function KeywordHandler:processNodeMessage(node, cid, message)
		local messageLower = string.lower(message)
		for i, childNode in pairs(node.children) do
			if(childNode:checkMessage(messageLower)) then
				local oldLast = self.getLastNode(cid)
				self.setLastNode(cid, childNode)
				childNode.parent = node -- Make sure node is the parent of childNode (as one node can be parent to several nodes).
				if(childNode:processMessage(cid, message)) then
					return true
				else
					self.setLastNode(cid, oldLast)
				end
			end
		end
		return false
	end
	
	-- Returns the root keywordnode
	function KeywordHandler:getRoot()
		return self.root
	end
	
	-- Adds a new keyword to the root keywordnode. Returns the new node. 
	function KeywordHandler:addKeyword(keys, callback, parameters)
		return self:getRoot():addChildKeyword(keys, callback, parameters)
	end
	
	-- Moves the current position in the keyword hierarchy count steps upwards. Count defalut value = 1.
	--	This function MIGHT not work properly yet. Use at your own risk.
	function KeywordHandler:moveUp(cid, count)
		local steps = count
		if(steps == nil) then
			steps = 1
		end
		local lastNode = self.getLastNode(cid)
		for i=1,steps,1 do
			if(lastNode == nil) then
				break
			else
				lastNode = lastNode:getParent() or self:getRoot()
			end
		end
		self.setLastNode(cid, lastNode)
		return self.lastNode
	end
	
	function KeywordHandler:sortKeywords()
		self:getRoot():sort()
	end
end
