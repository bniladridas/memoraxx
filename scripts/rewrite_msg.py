#!/usr/bin/env python3
import sys
import re

def rewrite_msg(msg):
    lines = msg.split('\n')
    first_line = lines[0]

    # Check if conventional
    match = re.match(r'^(feat|fix|docs|style|refactor|test|chore|perf|ci|build|revert)(\(.+\))?: ', first_line)
    if match:
        type_scope = match.group(1) + (match.group(2) if match.group(2) else '') + ':'
        desc = re.sub(r'^.*?: (.*)', r'\1', first_line).lower()
        new_msg = f"{type_scope} {desc}"
    else:
        new_msg = f"feat: {first_line.lower()}"

    # Truncate to 40 chars
    if len(new_msg) > 40:
        new_msg = new_msg[:37] + "..."

    return new_msg + '\n' + '\n'.join(lines[1:]) if len(lines) > 1 else new_msg

if __name__ == '__main__':
    msg = sys.stdin.read()
    print(rewrite_msg(msg))