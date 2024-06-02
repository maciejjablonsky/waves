param (
    [string]$RunnerName = $env:RUNNER_NAME,
    [string]$RepoUrl = $env:REPO_URL,
    [string]$Token = $env:TOKEN
)

# Print environment variables for debugging
Write-Output "RUNNER_NAME: $RunnerName"
Write-Output "REPO_URL: $RepoUrl"
Write-Output "TOKEN: $Token"

# Validate parameters
if (-not $RunnerName) { throw "RunnerName is required" }
if (-not $RepoUrl) { throw "RepoUrl is required" }
if (-not $Token) { throw "Token is required" }

# Function to unregister the runner
function Unregister-Runner {
    Write-Output "Unregistering runner..."
    & ./config.cmd remove --unattended --token $Token
    Write-Output "Runner unregistered"
}

# Function to handle termination signals
function Termination-Handler {
    Write-Output "Termination signal received, unregistering runner..."
    Unregister-Runner
    Write-Output "Runner unregistered, exiting..."
    exit 0
}

# Register WMI event to handle termination signals
$processId = [System.Diagnostics.Process]::GetCurrentProcess().Id
Register-WmiEvent -Query "SELECT * FROM __InstanceModificationEvent WITHIN 1 WHERE TargetInstance ISA 'Win32_Process' AND TargetInstance.ProcessId = $processId AND TargetInstance.TerminationDate IS NOT NULL" -Action {
    Termination-Handler
}

# Configure the GitHub Actions runner
Write-Output "Configuring runner..."
try {
    & ./config.cmd --url $RepoUrl --token $Token --name $RunnerName
} catch {
    Write-Error "Configuration failed: $_"
    exit 1
}

# Run the GitHub Actions runner
Write-Output "Starting runner..."
try {
    Start-Process -FilePath "./run.cmd" -NoNewWindow -Wait
} catch {
    Write-Error "Runner failed to start: $_"
    exit 1
}

# Wait indefinitely to handle the trap
while ($true) {
    Start-Sleep -Seconds 1
}
