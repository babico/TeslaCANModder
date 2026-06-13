# CI Fix & Repo Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix CI checkout failures and Dependabot errors by removing stale submodule references and cleaning up stale PRs/branches.

**Architecture:** Git-level cleanup: remove a dangling submodule tree entry, strip a dead submodule from `.gitmodules`, configure Dependabot to skip `firmware/`, then close all stale Dependabot PRs and delete their remote branches via GitHub API + git push.

**Tech Stack:** Git, GitHub API (gh cli), GitHub MCP tools

---

### Task 1: Remove stale submodule tree entry `legacy/tesla-fsd-can-mod-2-main`

**Files:**

- Modify: git index (no file changes)

- [ ] **Step 1: Remove from git index only**

```bash
git rm --cached legacy/tesla-fsd-can-mod-2-main
```

Expected: removes the tree entry from the index. No file deletion. `.gitmodules` unchanged (entry didn't exist there).

- [ ] **Step 2: Verify removal**

```bash
git ls-tree HEAD legacy/tesla-fsd-can-mod-2-main
```

Expected: no output (entry gone from index)

- [ ] **Step 3: Verify other submodules still intact**

```bash
git submodule status 2>&1 | Select-String "tesla-fsd-can-mod-2-main"
```

Expected: no output (no references remain)

---

### Task 2: Remove dead submodule `Shayennn/FUCKYOU-TESLA-FSD` from .gitmodules and strip .git

**Files:**

- Modify: `.gitmodules` (remove lines 236-238)
- Delete: `legacy/Shayennn-FUCKYOU-TESLA-FSD/.git`

- [ ] **Step 1: Remove submodule block from .gitmodules**

Edit `.gitmodules` to remove:

```
[submodule "legacy/Shayennn-FUCKYOU-TESLA-FSD"]
	path = legacy/Shayennn-FUCKYOU-TESLA-FSD
	url = https://github.com/Shayennn/FUCKYOU-TESLA-FSD
```

The neighboring entries are `[submodule "legacy/slxslx-tesla-open-can-mod-slx-repo"]` (ends at line 235) and `[submodule "legacy/EzeLLM-fsd-spoofing"]` (starts at line 239).

Use Edit tool:

- oldString: the exact 3-line block
- newString: empty

- [ ] **Step 2: Git rm the submodule from index (keep files)**

```bash
git rm --cached legacy/Shayennn-FUCKYOU-TESLA-FSD
```

Expected: removes submodule from index.

- [ ] **Step 3: Delete the inner .git directory (convert to plain files)**

```bash
Remove-Item -Recurse -Force -LiteralPath "legacy\Shayennn-FUCKYOU-TESLA-FSD\.git"
```

- [ ] **Step 4: Stage the de-submoduled files as regular files**

```bash
git add legacy/Shayennn-FUCKYOU-TESLA-FSD/
```

- [ ] **Step 5: Verify**

```bash
git submodule status 2>&1 | Select-String "Shayennn"
```

Expected: no output (no longer a submodule)

---

### Task 3: Create dependabot.yml to exclude firmware

**Files:**

- Create: `.github/dependabot.yml`

- [ ] **Step 1: Create .github directory**

```bash
New-Item -ItemType Directory -Force -Path ".github"
```

- [ ] **Step 2: Write dependabot.yml**

Write to `.github/dependabot.yml`:

```yaml
version: 2
updates:
    - package-ecosystem: "npm"
      directory: "/"
      schedule:
          interval: "weekly"
      open-pull-requests-limit: 10

    - package-ecosystem: "npm"
      directory: "/client"
      schedule:
          interval: "weekly"
      open-pull-requests-limit: 5

    - package-ecosystem: "npm"
      directory: "/packages/protocol"
      schedule:
          interval: "weekly"
      open-pull-requests-limit: 5

    - package-ecosystem: "npm"
      directory: "/tools"
      schedule:
          interval: "weekly"
      open-pull-requests-limit: 5

    - package-ecosystem: "docker"
      directory: "/hardware"
      schedule:
          interval: "monthly"
      open-pull-requests-limit: 3

    - package-ecosystem: "docker"
      directory: "/web"
      schedule:
          interval: "monthly"
      open-pull-requests-limit: 3
```

Note: `firmware/` is excluded because PlatformIO is not a supported Dependabot package-ecosystem.

- [ ] **Step 3: Stage**

```bash
git add .github/dependabot.yml
```

---

### Task 4: Close all open Dependabot PRs

Use GitHub MCP tools to close each open PR authored by `dependabot[bot]`.

- [ ] **Step 1: List all open PRs**

Use `github-mcp_list_pull_requests` with state=open. Filter to PRs where `user.login === "dependabot[bot]"`. Record each PR number.

- [ ] **Step 2: Close each PR**

For each Dependabot PR number, use `github-mcp_update_pull_request` with `state: "closed"`.

Example for PR #50:

```
github-mcp_update_pull_request(owner="babico", repo="TeslaCANModder", pullNumber=50, state="closed")
```

Repeat for all Dependabot PR numbers.

- [ ] **Step 3: Verify**

Use `github-mcp_list_pull_requests` with state=open. Expected: zero Dependabot PRs.

---

### Task 5: Delete all Dependabot remote branches

- [ ] **Step 1: List remote Dependabot branches**

```bash
git branch -r | Select-String "dependabot/"
```

- [ ] **Step 2: Delete each remote branch**

For each `origin/dependabot/*` branch:

```bash
git push origin --delete dependabot/<branch-name>
```

Can batch with a loop:

```powershell
git branch -r | Select-String "dependabot/" | ForEach-Object {
    $branch = $_.ToString().Trim() -replace "origin/", ""
    git push origin --delete $branch
}
```

- [ ] **Step 3: Prune local remote-tracking refs**

```bash
git remote prune origin
```

- [ ] **Step 4: Verify**

```bash
git branch -r | Select-String "dependabot/"
```

Expected: no output

---

### Task 6: Delete stale local branches

- [ ] **Step 1: List non-main local branches**

```bash
git branch | Select-String -NotMatch "main"
```

- [ ] **Step 2: Delete `copilot/fix-all-prs`**

```bash
git branch -D copilot/fix-all-prs
```

- [ ] **Step 3: Delete `fix/local-changes-batch`**

```bash
git branch -D fix/local-changes-batch
```

- [ ] **Step 4: Verify**

```bash
git branch
```

Expected: only `main` (or current branch) remains.

---

### Task 7: Commit and push

- [ ] **Step 1: Review staged changes**

```bash
git status
git diff --cached --stat
```

Expected staged:

- `.gitmodules` (modified: Shayennn entry removed)
- `.github/dependabot.yml` (new)
- `legacy/Shayennn-FUCKYOU-TESLA-FSD/` (added as regular files)
- `legacy/tesla-fsd-can-mod-2-main` (removed from index)

- [ ] **Step 2: Commit**

```bash
git add -A
git commit -m "fix: remove stale submodule refs, add dependabot config, close stale PRs"
```

- [ ] **Step 3: Push**

```bash
git push origin main
```

---

### Task 8: Verify CI passes

- [ ] **Step 1: Check GitHub Actions**

After push, verify the CI workflow at https://github.com/babico/TeslaCANModder/actions runs succeed (no submodule checkout errors).

- [ ] **Step 2: Verify Dependabot**

Check https://github.com/babico/TeslaCANModder/network/updates — no errors for `firmware/` or submodule cloning.

- [ ] **Step 3: Verify submodule status locally**

```bash
git submodule status
```

Expected: no fatal errors, all listed submodules resolve.
