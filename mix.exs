defmodule Mix.Tasks.Compile.NIF do
  @shortdoc "Compiles sass library"
  def run(_) do
    if Mix.shell.cmd("make nif") != 0 do
      raise Mix.Error, message: "could not run `make nif`."
    end
  end
end

defmodule Cbson.Mixfile do
  use Mix.Project

  def project do
    [app: :cbson,
     version: "0.0.1",
     elixir: "~> 1.1",
     build_embedded: Mix.env == :prod,
     start_permanent: Mix.env == :prod,
     compilers: [:NIF, :elixir, :app, :erlang],
     deps: deps]
  end

  # Configuration for the OTP application
  #
  # Type "mix help compile.app" for more information
  def application do
    [applications: [:logger]]
  end

  # Dependencies can be Hex packages:
  #
  #   {:mydep, "~> 0.3.0"}
  #
  # Or git/path repositories:
  #
  #   {:mydep, git: "https://github.com/elixir-lang/mydep.git", tag: "0.1.0"}
  #
  # Type "mix help deps" for more examples and options
  defp deps do
    [
      {:bson, "~> 0.4.0", only: :test},
    ]
  end
end
