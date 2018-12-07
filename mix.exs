defmodule Mix.Tasks.Compile.Nif do
  def run(_) do
    cp = Mix.Project.compile_path()
    priv_path = Path.join([cp, "..", "priv"])
    File.mkdir_p(priv_path)
    File.mkdir_p("priv")
    if Mix.shell.cmd("PRIV_PATH=#{priv_path} make nif") != 0 do
      raise Mix.Error, message: "could not run `make nif`."
    end
    File.copy(Path.join([priv_path, "bson_nif.so"]), "priv")
    :ok
  end
end

defmodule Cbson.Mixfile do
  use Mix.Project

  def project do
    [app: :cbson,
     version: "0.0.5",
     elixir: "~> 1.7",
     build_embedded: Mix.env == :prod,
     start_permanent: Mix.env == :prod,
     compilers: [:nif | Mix.compilers],
     deps: deps()]
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
      {:benchfella, "~> 0.3.0", only: :bench},
    ]
  end
end
