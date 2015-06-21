# coding: utf-8
lib = File.expand_path('../ruby/lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'rugged/postgresql/version'

Gem::Specification.new do |spec|
  spec.name          = "rugged-postgresql"
  spec.version       = Rugged::Postgresql::VERSION
  spec.authors       = ["David Albert"]
  spec.email         = ["davidbalbert@gmail.com"]

  spec.summary       = %q{PostgreSQL storage backend for Rugged}
  spec.description   = %q{Store a bare git repository in a PostgreSQL database}
  spec.homepage      = "https://github.com/davidbalbert/libgit2-postgresql"
  spec.license       = "GPL v3"

  spec.files         = `git ls-files -z`.split("\x0").reject { |f| f.match(%r{^(test|spec|features)/}) }
  spec.bindir        = "ruby/exe"
  spec.executables   = spec.files.grep(%r{^exe/}) { |f| File.basename(f) }
  spec.require_paths = ["ruby/lib"]
  spec.extensions = ["ruby/ext/rugged/postgresql/extconf.rb"]

  spec.add_dependency "rugged", "~> 0.22"

  spec.add_development_dependency "bundler", "~> 1.9"
  spec.add_development_dependency "rake", "~> 10.0"
  spec.add_development_dependency "rake-compiler"
  spec.add_development_dependency "pry"
end
