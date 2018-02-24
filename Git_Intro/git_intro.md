
Git Introduction
=====================

A study notes for Git. (The raw draft is written using **StackEdit Editer**[^stackedit].)

>[TOC]

---------

## Git Basics ##

### Git is Distributed Version Control Systems (DVCS) ###

 - **No special server distribution**

	A Git distribution is both sever and client inherent. A Git machine, can either act as a server which host repositories for other Git machine to check in/out or act as a client which update/commit code from/to any accessible Git machines include itself.

 - **Git fully mirror the repository**

	Checking out a subtree of a repository is not allowed in Git. While checkout, Git clones the entire repository, including history.

 - **Repository History in Git is Integral**

	Git not just check out the latest snapshot of the files, but the entire history of the repository.

 - **Nearly Every Operation Is Local**
	
	Since every checkout is really a full backup of all the data. Git doesn't need to go out to the server to get the history and thus enables the committing and history displaying operations locally.

 - **Git Has no Access Control**

	Due to it's distributed property, Git inherently do not have Access Control features. But which may be possible by external tools such as *gitosis* or *gitolite*.

### Git is a content-addressable file system ###

Nearly everything in Git is check-summed (into a 40 bytes SHA–1 value) and then stored in the name of that checksum. These content-checksum-named files are called **hash objects**. 

*Blob*, *tree* and *commit* are the most important hash objects:

 - ***blobs*** and ***trees*** are used for restoring the file system
 - ***commits*** are used for tracing the developing history.

#### Blob & Tree Objects ####

Git stores content in a manner similar to a UNIX file system, but a bit simplified. All the content is stored as tree and blob objects, with *trees* corresponding to UNIX directory entries and *blobs* corresponding to inodes or file contents.

Both directory name and file name are resolved by tree objects. A single tree object contains one or more tree entries, each of which contains an SHA–1 pointer to a blob or subtree with its associated mode, type, and filename. For example:

```shell
	$ git cat-file -p master?{tree}
	100644 blob a906cb2a4a904a152e80877d4088654daad0c859 README
	100644 blob 8f94139338f9404f26296befa88755fc2598c289 Makefile
	040000 tree 99f1a6d12cb4b6f19c8655fca46c3ecf317074e0 lib
```

#### Commit Objects ####

A typical commit object include these items: 

 - top-level tree for the snapshot of the entire project at the commit point
 - parents: commits that right before the current commit
 - the author/committer information with the current time stamp
 - the commit message follow a blank line

Here is an example:

```shell
	$ git cat-file commit ca82a6
	tree cfda3bf379e4f8dba8717dee55aab78aef7f4daf
	parent 085bb3bcb608e1e8451d4b2432f8ecbe6306e7e7
	author Scott Chacon <schacon@gmail.com> 1205815931 -0700
	committer Scott Chacon <schacon@gmail.com> 1240030591 -0700
	
	changed the verison number
```

- Commits may have no parents, first commit e.g.
- Normal commits have just one parent
- Merged commits may have multiple parents.

####What is content-addressable####

Suppose you repo have multi copy of GPLv3 LICENSE in different directories. Git make just one storage. 
```
    repo_root
    ├───COPYING.GPLv3
    ├───aDir
    │   └───COPYING.GPLv3
    ├───bDir
    │   └───COPYING.GPLv3
    └───cDir
        └───COPYING.GPLv3
```
The directories of *aDir*, *bDir* and *cDir* are all just include a GPLv3 license with the same file-name. In other word, they has exactly the same contents. So git just store one tree-object. Beside, since all the GPLv3 licenses are same, git just make one blob-object to store it.

```
    $ git cat-file -p master^{tree}
    100644 blob 94a9ed024d3859793618152ea559a168bbcbb5e2    COPYING.GPLv3
    040000 tree 3e3585b5429d23870ad92f4564f277213c1d65e3    aDir
    040000 tree 3e3585b5429d23870ad92f4564f277213c1d65e3    bDir
    040000 tree 3e3585b5429d23870ad92f4564f277213c1d65e3    cDir
    $ git cat-file -p 3e3585b5
    100644 blob 94a9ed024d3859793618152ea559a168bbcbb5e2    COPYING.GPLv3
```

What if we just change the file-name for GPLv3 license without other modification.

```
    $ mv aDir/COPYING.GPLv3 aDir/COPYING.GPLv3.a
    $ mv bDir/COPYING.GPLv3 aDir/COPYING.GPLv3.b
    $ mv cDir/COPYING.GPLv3 aDir/COPYING.GPLv3.c
    $
    $ git add --all
    $ git status
    # On branch master
    # Changes to be committed:
    #   (use "git reset HEAD <file>..." to unstage)
    #
    #       renamed:    aDir/COPYING.GPLv3 -> aDir/COPYING.GPLv3.a
    #       renamed:    bDir/COPYING.GPLv3 -> bDir/COPYING.GPLv3.b
    #       renamed:    cDir/COPYING.GPLv3 -> cDir/COPYING.GPLv3.c
    #
```

Amazingly, without been told, git figured out that we just renamed the file.

```
    $ git cat-file -p master^{tree}
    100644 blob 94a9ed024d3859793618152ea559a168bbcbb5e2    COPYING.GPLv3
    040000 tree ba1d6442bdf21e07fe852cfb7d049b88c1e7b476    aDir
    040000 tree 44ed552fd68b312e6420ec5c1a3cc763013ff131    bDir
    040000 tree 78caf0d40acd117ba1a220ec700385601facffe1    cDir
    $ git cat-file -p ba1d6442b
    100644 blob 94a9ed024d3859793618152ea559a168bbcbb5e2    COPYING.GPLv3.a
    $ git cat-file -p 44ed552f
    100644 blob 94a9ed024d3859793618152ea559a168bbcbb5e2    COPYING.GPLv3.b
    $ git cat-file -p 78caf0d4
    100644 blob 94a9ed024d3859793618152ea559a168bbcbb5e2    COPYING.GPLv3.c
```

> `git mv old new` =  `mv old new` + `git add old` + `git add new`

### Commits History in Git is non-linear ###

CVSs such as SVN often maintain a linear history: commits are arranged in series, even though develop may happened in parallel.

While in Git, it is not just allowed but often in common case, the commits history is non-linear. A typical Git History, in presentation, is a **directed acyclic graph (DAG)** like this:

![history tree layout](https://raw.github.com/gitgjogh/Git_Intro/master/Figures/Git_History_Sample.png)

>Sometimes, for conveniency, we will represent Git history in plain text diagrams like the one below.
>
>                  branch_1
>                     |
>             A---B---C---J  branch_2
>            /         \
>       D---E---F---G---H---I  master (HEAD)
>                \
>                 K---L---M  branch_3
>
> - **Commits** are shown as single capital characters.
> - **Branches** are represented in lowercase words.
> - **links** between commits with lines drawn by '-', '/', '\'. 
> - **time** goes left to right for a single branch

### Branch Is Just Symbolic Reference Point to Head Commit ###

When we talk about branch, to be precise, we imply linked list form by searial commits in time sequence. In the example above, there are 4 branches in the repo, they are:

        branch_1: C -> B -> A -> D
        
        branch_2: J -> C -> B -> A -> D
        
                          C -> B -> A
                        /            \
        master:   I -> H -> G -> F -> E -> D
        
        branch_3: M -> L -> K -> F -> E -> D

While **"branch head"** or **"head commit"** is the last commit in a branch (Here C,J,I,M). In git, we sometimes confuse branch with branch head. Because Git don't store the branch's history from its first commit to its head. Git just represent a branch by using a *symbolic reference* point to the the branch's head commit.

A **“reference”** or **“ref”** is a human-frendly name file that store a commit's SHA-1 value. Branch (head) is just one kind of refs. Other refs inlude *tag*, *remote branch*. You can find them in `.git/refs` directory:

>   |    refs         | location             |
>   | --------------: | :----------------    |
>   | branches head   | `.git/refs/heads/`   |
>   | tags            | `.git/refs/tags/`    |
>   | remote branches | `.git/refs/remotes/` |

branches head
:    Git system will auto-update the SHA–1 value to the last commit for your working branch.

tags
:   Tags never move. Tags are used for marking the important commits, for example, some milestones like 1.0 or 2.0. 

remote branches
:   Remote branch acts as bookmark to remind you the branch state on remote repo the last time you connected to that remote repo.

    You can’t checkout or move the remote branches. Git moves them automatically when you sync with remote repos. These will be talked later.

## Git Branches ##

### Branch Is The Beginning For History Trace ###

Since Git don't store any clues for branch history directly. The only way to recover a branch or the entire repo's history is recursively looking up each commit's parents. Branches are just the start points the history traversal begin at.

![history traversal](https://raw.github.com/gitgjogh/Git_Intro/master/Figures/Git_History_Retrospection.png)

The entire repo's history, in visualized graph, is some directed acyclic graph somehow like a tree or forest. Branches (head) need not to be leafs. But leafs must be branches (head), or else some history may loose (see detached HEAD branch). 

### HEAD : HEAD Branch or detached HEAD ###

When you have many commits and branches in repo, how does Git know which branch you are working on and which commit the subsequent work is basing on. The answer is the HEAD file. 

The `.git/HEAD` file in normal is a reference to the branch you’re currently on. The branch recored in HEAD file branch is usally called **HEAD branch** or **working branch**. When you checkout and switch to other branch, Git will update HEAD file to the just HEAD branch automatically. 

```shell
    $ cat .git/HEAD
    ref: refs/heads/master
    $ git checkout -b branch_1 && cat .git/HEAD
    ref: refs/heads/branch_1
    $ git checkout master && cat .git/HEAD
    ref: refs/heads/master
```

**Detached HEAD**

Checking out a commit dirctly other than a branch is allowed in Git. In this situation, HEAD is detached from any existing branches and points to the SHA-1 value of that commit directly. This is called *detached HEAD*. 

```shell
    $ git checkout dcd215b
    $ cat .git/HEAD
    dcd215bdcd215bdcd215bdcd215bdcd215b
```
When you run `git commit`:

 - **If HEAD points to a branch**
    
    Git will find the working branch HEAD points to first, then specify the parent of the new commit with the SAH-1 value stored in that working branch. Finally, update working branch to the new commit object. HEAD file itself needs not to be changed.

 - **If HEAD points to a commit**
    
    Git will specify the parent of the new commit with the SAH-1 value stored in HEAD. Then the SAH-1 value stored in HEAD file would be updated to the new commit object. No symbolic branch is changed.

**Detached HEAD is not encouraged!**

It’s easy to lose changes if you work on the detached HEAD. After you make several commits in a detached HEAD environment, you maked a "detached" branch. If you then carelessly switch to an existing branch, these commits may loose[^restore]. 

Any time you work on a detached branch, you can run the `git branch` command to save the detached HEAD with a newly created branch and switch to the new branch.

> In Git, Branch is really lightweight. Creating or deleting a branch is easy both for you and the Git, do it any time you need! 

### Branch Merging ###

The merge work in Git, essentially, is incorporating changes from other branches into HEAD branch. Or more fundamental, merging other branches' head commit into HEAD commit. 

Here is an example that merge branch_1 into HEAD (on master at #G before merge): 

```
          A---B---C  branch_1                     A---B---C  branch_1
         /                                       /         \
    D---E---F---G  master (HEAD)            D---E---F---G---H  master (HEAD)
```

As can be seen, `git merge branch_1` just do the work *"merge commit #C & #G into a automatically created commit object #H"*. More detailly, 

1. Git will first find the common ancestor (#E) for the merge brahches (#C and #G).
2. Git creates a new snapshot that results from three-way (#C-#E-#G) merge.
3. Git automatically creates a new commit (#H) that points to the new snapshot.<br>
    The new commit object (#H) would be specified both #C and #G as its parents.
4. HEAD branch is update to the new commit object #H automatically like normal commit operation.

####  **[Three-way Merge](http://en.wikipedia.org/wiki/Merge_(revision_control)#Three-way_merge "Wikipedia")** ####

A three-way merge is performed after an automated difference analysis between 2 modified derivatives (#C and #G) while also considering the common ancestor (#E) of the modified derivatives:

- Blocks that have changed in neither are left as they are. 
- Blocks that have changed in only one derivative use that changed version.
- If a block is changed in both derivatives, it is marked as a conflict situation and left for the user to resolve.

```
      common                           Three-way Merge
     ancestor                        (#C--#E--#G) ==> #H
        |                         
        | A---B---C  branch_1               E~~~c
        |/                                   \   \
    D---E---F---G  master                     G~~~H   
```

Automatic Merge
:   All the blocks are unchanged or just changed one derivatives.

Merge Fail: Conflict
:   There are blocks changed in both derivatives. User has to resolve the confilcts manually.

#### Fast-forward merge ####

When the head of the branch to be merged (into HEAD) is derived from HEAD. Merge work can be proceed by simply fast-forward. 

For example, after merge branch_1 into master, and then commit with #I, the head of master (#I) now is a ancestor for the head of branch_1 (#C). If we then merge master into branch_1

>             A---B---C  branch_1
>            /         \
>       D---E---F---G---H---I  master (HEAD)
>
>`$ git checkout branch_1`
>
>             A---B---C  branch_1 (HEAD)
>            /         \
>       D---E---F---G---H---I  master
>
>`$ git merge master`
>
>             A---B---C
>            /         \
>       D---E---F---G---H---I  master
>                              branch_1 (HEAD)
>
>Git simply move branch_1 from #C to #I.

## Remote Branches ##

Git provide 3 cmd to sync with remote branches.

>Pull & Push is not sync cmd, they are branch merge cmds.

fetch
:   Download objects and refs from another repository

pull
:   Fetch a remote branch, then merge it into a local branch

push
:   Upload a local branch, then merge it into a remote branch. 
:   In normal, **only fast-foreward merge is allowed.**

### Fetch ###

```
github:
              A---B---C---J  branch_2
             /         \
        D---E---F---G---H---I  master
                 \
                  K---L---M  branch_3
local:
              A---B---C---O  branch_2
             /         \
        D---E---F---G---H---I  master
```
After `git fetch`, the local history would like this
```
local:
                        J  remotes/github/branch_2
                       /
              A---B---C---O  branch_2
             /         \
        D---E---F---G---H---I  master & remotes/github/master
                 \
                  K---L---M  remotes/github/branch_3
```

### Pull ###

```
github:
              A---B---C---J  branch_2
             /         \
        D---E---F---G---H---I  master
                 \
                  K---L---M  branch_3
local:
              A---B---C---O  branch_2
             /         \
        D---E---F---G---H---I  master
```
After `git pull github branch_2:branch_2`, the local history would like this
```
local:
            remotes/github/branch_2
                        |
                        J---M  <-branch_2
                       /   / 
              A---B---C---O 
             /         \
        D---E---F---G---H---I  master & remotes/github/master
                 \
                  K---L---M  remotes/github/branch_3
```
### Push ###
IF you want to push a local branch to a remote branch that need three-way merge, you can pull firet, and then push. 

```
github:
        D---E---A---B---C---J  branch_2
local:
        D---E---A---B---C---O  branch_2
```

`git push github branch_2:branch_2` would be refused by Git at this point. You need `git pull github branch_2:branch_2` first

```
local:
               remotes/github/branch_2
                          |
                          J---M  <-branch_2
                         /   / 
        D---E---A---B---C---O 
```

Then `git pull github branch_2:branch_2` would be ok

```
github:
                          J---M  <-branch_2
                         /   / 
        D---E---A---B---C---O 
```


[^reflog]: Reflog is not belong to repo.

[^restore]: You can use reflog in `.git/logs/HEAD` to help you restore the data.

[^stackedit]: StackEdit is a free, open-source Markdown editor based on PageDown, the Markdown library used by Stack Overflow and the other Stack Exchange sites.

  [1]: ./figures/history_display.png