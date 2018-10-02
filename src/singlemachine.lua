_instancia=1

function le_dados(filename)
  local base={}
  for l in io.lines(filename) do
    --#Inst	Ord	Rj	Dj	Wj	WTj	WEj	Pj
	--print(l)
	--local i, o = l:match '(%S+)%s+(%S+)'
    --table.insert(base, { inst = i, ord = o})
	local i, o, r, d, w, wt, we, p = l:match '(%S+)%s+(%S+)%s+(%S+)%s+(%S+)%s+(%S+)%s+(%S+)%s+(%S+)%s+(%S+)'
	--if (i:match '#.*' == nil) then
      table.insert(base, { inst = i, ord = o, rj = r, dj = d, wj = w, wtj = wt, wej = we, pj = p})
	--end
  end
  return base
end

tabela = le_dados('dados.txt')

for k, v in ipairs(tabela) do
    print(v.inst,v.ord,v.rj,v.dj,v.wj,v.wtj,v.wej,v.pj)
end
