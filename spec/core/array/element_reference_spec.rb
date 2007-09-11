require File.dirname(__FILE__) + '/../../spec_helper'
require File.dirname(__FILE__) + '/fixtures/classes'
require File.dirname(__FILE__) + '/slice_spec'

describe "Array.[]" do
  it "returns a new array populated with the given elements" do
    Array.[](5, true, nil, 'a', "Ruby").should == [5, true, nil, "a", "Ruby"]

    a = MyArray.[](5, true, nil, 'a', "Ruby")
    a.class.should == MyArray
    a.inspect.should == [5, true, nil, "a", "Ruby"].inspect
  end
end

describe "Array[]" do
  it "is a synonym for .[]" do
    Array[5, true, nil, 'a', "Ruby"].should == Array.[](5, true, nil, "a", "Ruby")

    a = MyArray[5, true, nil, 'a', "Ruby"]
    a.class.should == MyArray
    a.inspect.should == [5, true, nil, "a", "Ruby"].inspect
  end
end

describe "Array#[]" do
  it_behaves_like(@array_slice, :[])
end
