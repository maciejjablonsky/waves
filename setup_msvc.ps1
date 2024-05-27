param(
    [switch]$installOnly
)

Write-Host "Running Conan install..."
conan install . -pr:h dev-msvc -b missing -s build_type=Release --lockfile=conan.lock
conan install . -pr:h dev-msvc -b missing -s build_type=Debug --lockfile=conan.lock

if ($installOnly) {
    exit 0
}

# Check if the previous command was successful
if ($LASTEXITCODE -ne 0) {
    Write-Error "Conan install failed."
    exit $LASTEXITCODE
}

Write-Host "Executing the bat script and opening Visual Studio..."
cmd /c "build\generators\conanbuild.bat && devenv ."
