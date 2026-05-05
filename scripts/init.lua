local command = require('core.command')

-- The C++ Engine will read AddonManager's manifest and automatically load 
-- the 15 required services in the perfect topological order!
command.input('/load AddonManager')