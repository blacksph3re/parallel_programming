defmodule Example do
	def child(parent) do
		receive do
			x -> send parent, :math.pow(x, :math.pi)
		end
	end
end

parent = self()

1..1000
	|> Enum.map(fn(x) -> {x, (spawn fn -> Example.child(parent) end)} end)
	|> Enum.map(fn({x, pid}) -> {x, (send pid, x)} end)

1..1000
	|> Enum.map(fn(_) -> (receive do x -> x end) end)
	|> IO.inspect


1..1000
	|> Enum.map(&(Task.async(fn -> :math.pow(&1, :math.pi) end)))
    |> Enum.map(&Task.await(&1))
    |> IO.inspect
