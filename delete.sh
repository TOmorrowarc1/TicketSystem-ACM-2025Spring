git rev-list --objects --all | \
  git cat-file --batch-check='%(objecttype) %(objectname) %(objectsize) %(rest)' | \
  awk -v limit=100000000 '/^blob/ && $3 > limit {print $4}' > large_files.txt

# 从历史中删除这些文件
cat large_files.txt | while read file; do
  git filter-branch --force --index-filter \
    "git rm --cached --ignore-unmatch '$file'" \
    --prune-empty --tag-name-filter cat -- --all
done