#!/bin/bash

# Rewrite commit message: lowercase, truncate to 40 chars, ensure conventional

read -r FIRST_LINE

# If already conventional, just lowercase and truncate
if echo "$FIRST_LINE" | grep -qE '^(feat|fix|docs|style|refactor|test|chore|perf|ci|build|revert)(\(.+\))?: '; then
    # Lowercase the description part
    TYPE_SCOPE=$(echo "$FIRST_LINE" | sed -E 's/^(feat|fix|docs|style|refactor|test|chore|perf|ci|build|revert)(\(.+\))?: .*/\1\2:/')
    DESC=$(echo "$FIRST_LINE" | sed -E 's/^(feat|fix|docs|style|refactor|test|chore|perf|ci|build|revert)(\(.+\))?: (.*)/\3/' | tr '[:upper:]' '[:lower:]')
    NEW_MSG="$TYPE_SCOPE $DESC"
else
    # If not, assume it's a description, make it feat: lowercase
    NEW_MSG="feat: $(echo "$FIRST_LINE" | tr '[:upper:]' '[:lower:]')"
fi

# Truncate to 40 chars
if [ ${#NEW_MSG} -gt 40 ]; then
    NEW_MSG="${NEW_MSG:0:37}..."
fi

echo "$NEW_MSG"
