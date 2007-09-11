require File.dirname(__FILE__) + '/../../spec_helper'
require File.dirname(__FILE__) + '/fixtures/classes'

describe "Module#const_get" do
  it "returns the value of the constant with the given name" do
    ModuleSpecs.const_get(:BasicModule).should == ModuleSpecs::BasicModule
    ModuleSpecs.const_get(:SuperModule.to_i).should == ModuleSpecs::SuperModule
    ModuleSpecs.const_get("Child").should == ModuleSpecs::Child
  end
  
  it "raises a NameError when there is no constant with the given name" do
    should_raise(NameError, "uninitialized constant ModuleSpecs::NotExistant") do
      ModuleSpecs.const_get("NotExistant").should == nil
    end
  end

  it "tries to convert the given name to a string using to_str" do
    (o = Object.new).should_receive(:to_str, :returning => "Parent")
    ModuleSpecs.const_get(o).should == ModuleSpecs::Parent
  end

  it "raises a TypeError when the given name can't be converted to string using to_str" do
    o = Object.new
    should_raise(TypeError, "#{o} is not a symbol") do
      ModuleSpecs.const_get(o)
    end

    o.should_receive(:to_str, :returning => 123)
    should_raise(TypeError, "Object#to_str should return String") do
      ModuleSpecs.const_get(o)
    end
  end
end
