## git subtree cheat sheet

### Add an external subtree

    git subtree add --prefix <path> <url> <commit/branch> --squash

### Update an external subtree (quick)

    git subtree pull --prefix <path> <url> <commit/branch> --squash

### Add as remote

    git remote add -f <remote-name> <url>

### Update an external subtree

    git fetch <remote-name>
    git subtree pull --prefix <path> <remote-name> <commit/branch> --squash

### Push an external subtree

    push --prefix=<path> <remote-name> <branch>
