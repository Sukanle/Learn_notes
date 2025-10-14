-- .nvim.lua
vim.api.nvim_create_autocmd("VimEnter", {
  pattern = "main.cpp",
  callback = function()
    vim.cmd("botright vsplit /tmp/benched.cpp | set ft=cpp")
    vim.cmd("split /tmp/watched.s | set ft=nasm")
  end,
})

