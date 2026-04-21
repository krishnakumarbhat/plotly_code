-- Initialize HPC Tools Database
-- This script runs when PostgreSQL container starts for the first time

-- Create extensions
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- Grant privileges
GRANT ALL PRIVILEGES ON DATABASE hpc_tools_db TO hpc_user;

-- Create tables will be handled by Flask-Migrate/SQLAlchemy
-- But we can add some initial setup here

-- Create indexes for performance (these will be created by SQLAlchemy models, 
-- but we add them here as well for explicit documentation)

-- Note: The actual table creation is handled by Flask's db.create_all() 
-- or Flask-Migrate, so this file is mainly for any database-level 
-- configuration that needs to happen before the application starts.

-- Set default timezone
SET timezone = 'UTC';

-- Log that initialization is complete
DO $$
BEGIN
    RAISE NOTICE 'HPC Tools database initialized successfully';
END $$;
