#!/bin/bash
# Final cleanup and push

cd /d/NextCloud/3D_NAS_Code/BeeHive/V2/dev

echo "🧹 Cleanup..."

# Remove old scripts
rm -f create_full_structure.sh
rm -f create_structure.sh
rm -f INSTALL.md

# Check .gitignore
if ! grep -q "secrets.h" .gitignore; then
    echo "include/secrets.h" >> .gitignore
fi

if ! grep -q ".pio" .gitignore; then
    echo ".pio/" >> .gitignore
fi

if ! grep -q ".vscode" .gitignore; then
    echo ".vscode/" >> .gitignore
fi

echo "✅ Cleanup done"

# Status
echo ""
echo "📊 Current state:"
git status --short

# Commit
echo ""
read -p "Commit and push? (y/n): " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "Cancelled"
    exit 0
fi

# Add all
git add .

# Commit
git commit -m "feat: final modular structure

Structure:
- firmware/common: sensors, sd_card, battery, bot
- firmware/gsm: main, modem, gps
- firmware/wifi: main, wifi_module, server, portal
- test: preserved working tests
- secrets in include/secrets.h (not committed)

Ready for development"

echo ""
echo "🚀 Pushing to all branches..."

# Push current branch
CURRENT=$(git branch --show-current)
git push -u origin $CURRENT

# Push other branches if exist
for branch in development feature/gsm-implementation feature/wifi-implementation testing; do
    if git show-ref --verify --quiet refs/heads/$branch; then
        echo "Pushing $branch..."
        git push -u origin $branch 2>/dev/null || echo "  Already up to date"
    fi
done

echo ""
echo "✅ Done!"
echo ""
echo "Branches on GitHub:"
git branch -r

