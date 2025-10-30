# 📤 GitHub Upload Guide - Step by Step

Follow these steps to upload your project to GitHub and make it look professional!

## 🎯 Prerequisites

You need:
- A GitHub account (if you don't have one, go to https://github.com and sign up)
- Git installed on your Mac (check by running `git --version` in Terminal)

## 📝 Step-by-Step Instructions

### Step 1: Install Git (if needed)

Open Terminal and check if Git is installed:

```bash
git --version
```

If not installed, macOS will prompt you to install it. Click "Install" and wait for it to complete.

### Step 2: Configure Git (First Time Only)

Tell Git who you are:

```bash
git config --global user.name "Your Name"
git config --global user.email "your.email@example.com"
```

Replace with your actual name and email (use the same email as your GitHub account).

### Step 3: Create a GitHub Repository

1. Go to https://github.com
2. Click the **"+"** button in the top-right corner
3. Select **"New repository"**
4. Fill in the details:
   - **Repository name**: `reazione-a-catena` (or any name you prefer)
   - **Description**: "Nuclear chain reaction simulator using multi-process IPC"
   - **Public** or **Private**: Choose based on your preference
   - ⚠️ **IMPORTANT**: Do NOT check "Initialize with README" (we already have one!)
5. Click **"Create repository"**

GitHub will show you a page with instructions. Keep this page open!

### Step 4: Prepare Your Project

Open Terminal and navigate to your project:

```bash
cd /Users/alexcalin/Desktop/progetto
```

Clean up build artifacts before uploading:

```bash
make clean
```

### Step 5: Initialize Git in Your Project

```bash
# Initialize git repository
git init

# Add all files
git add .

# Create your first commit
git commit -m "Initial commit: Nuclear chain reaction simulator"
```

### Step 6: Connect to GitHub

Go back to the GitHub page from Step 3. Look for the section that says:
**"…or push an existing repository from the command line"**

You'll see commands like this (YOUR URL will be different):

```bash
git remote add origin https://github.com/YOUR_USERNAME/reazione-a-catena.git
git branch -M main
git push -u origin main
```

**Copy those exact commands from YOUR GitHub page** and paste them into Terminal.

### Step 7: Enter GitHub Credentials

When you run `git push`, GitHub will ask for authentication:

1. **Username**: Your GitHub username
2. **Password**: You need to use a **Personal Access Token** (not your GitHub password!)

#### How to Create a Personal Access Token:

1. Go to https://github.com/settings/tokens
2. Click **"Generate new token"** → **"Generate new token (classic)"**
3. Give it a name: "Mac Terminal Access"
4. Set expiration: Choose what you prefer (e.g., 90 days)
5. Check the **"repo"** scope (this gives full control of repositories)
6. Scroll down and click **"Generate token"**
7. **COPY THE TOKEN** (you won't see it again!)
8. Use this token as your password when Git asks

### Step 8: Verify Upload

1. Go to your GitHub repository page: `https://github.com/YOUR_USERNAME/reazione-a-catena`
2. You should see all your files!
3. The README.md will be automatically displayed below the file list

## ✨ Making It Look Professional

### Add a License File

```bash
# Create MIT License file
cat > LICENSE << 'EOF'
MIT License

Copyright (c) 2024 Your Name

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
EOF

# Add and commit the license
git add LICENSE
git commit -m "Add MIT License"
git push
```

### Add Repository Topics

On your GitHub repository page:
1. Click the **⚙️ Settings** icon (or find "About" section on the right)
2. Click **"⚙️"** next to "About"
3. Add topics: `c`, `operating-systems`, `ipc`, `multi-processing`, `macos`, `fork`, `semaphores`, `shared-memory`
4. Save changes

### Add a Repository Description

In the same "About" section:
- Add description: "Nuclear chain reaction simulator using multi-process IPC mechanisms"
- Add website: (leave blank or add a link if you have one)

## 🔄 Making Updates Later

After making changes to your code:

```bash
# Check what changed
git status

# Add all changes
git add .

# Commit with a message
git commit -m "Description of what you changed"

# Push to GitHub
git push
```

## 📸 Add Screenshots (Optional but Impressive!)

1. Run your simulation and take a screenshot:
   ```bash
   ./run_timeout.sh
   # Press Cmd+Shift+4, then Space, then click the Terminal window
   ```

2. Create a screenshots folder:
   ```bash
   mkdir screenshots
   mv ~/Desktop/Screen*.png screenshots/demo.png
   ```

3. Reference it in README (edit README.md and add):
   ```markdown
   ## 📸 Screenshots

   ![Simulation Demo](screenshots/demo.png)
   ```

4. Commit and push:
   ```bash
   git add screenshots/
   git add README.md
   git commit -m "Add demo screenshot"
   git push
   ```

## 🎨 GitHub Will Automatically Show:

- ✅ Your beautiful README.md with emoji and formatting
- ✅ Badges at the top (the blue/green shields)
- ✅ Table of contents with clickable links
- ✅ Syntax-highlighted code blocks
- ✅ File tree browser
- ✅ Commit history

## 🆘 Troubleshooting

### "Permission denied"
- Make sure you're using a Personal Access Token, not your password

### "Repository not found"
- Check that the repository name matches exactly
- Make sure the URL is correct (copy from GitHub)

### "refusing to merge unrelated histories"
```bash
git pull origin main --allow-unrelated-histories
git push
```

### Want to start over?
```bash
rm -rf .git
# Then go back to Step 5
```

## 🎉 You're Done!

Your project is now on GitHub! Share the link:
`https://github.com/YOUR_USERNAME/reazione-a-catena`

The README will look professional with:
- Nice formatting
- Emoji icons
- Tables
- Code blocks with syntax highlighting
- Clickable table of contents
- Badges

---

**Need help?** Open an issue on your repository or ask for help!
