
print("Fibonacci solver")

function fib(i)
{
  if (i == 0 || i == 1)
    return 1
  else
    return fib(i - 1) * fib(i - 2)
}

print("Fib(20) = ")
print(fib(20))
