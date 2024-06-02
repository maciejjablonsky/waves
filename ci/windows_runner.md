# Setup

Build image with

```powershell
docker build -f .\ci\win_runner.Dockerfile -t windows-github-runner ci
```

Retrieve runner token from [new runner](https://github.com/maciejjablonsky/waves_field/settings/actions/runners/new)

Run container with

```powershell
docker run --name <runner_name> -e RUNNER_NAME=<runner_name> -e REPO_URL=https://github.com/maciejjablonsky/waves_field -e TOKEN=<runner_token> windows-github-runner
```
