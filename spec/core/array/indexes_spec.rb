require File.dirname(__FILE__) + '/../../spec_helper'
require File.dirname(__FILE__) + '/fixtures/classes'

@array_indexes = shared "Array#indexes" do |cmd|
  describe "Array##{cmd}" do
    it "returns elements at integer argument indexes (DEPRECATED)" do
      array = [1, 2, 3, 4, 5]

      x = Object.new
      def x.to_int() 4 end

      params = [1, 0, 5, -1, -8, 10, x]
      array.send(cmd, *params).should == array.values_at(*params)
    end
    
    it "calls to_int on arguments" do
      array = [1, 2, 3, 4, 5]

      x = Object.new
      def x.to_int() 4 end
      array.send(cmd, x).should == [5]

#      x = Object.new
#      x.should_receive(:respond_to?, :count => 2, :with => [:to_int], :returning => true)
#      x.should_receive(:method_missing, :count => 2, :with => [:to_int], :returning => 1)
#      array.send(cmd, x).should == array.send(cmd, x)
    end

    it "returns elements in range arguments as nested arrays (DEPRECATED)" do
      array = [1, 2, 3, 4, 5]
      params = [0..2, 1...3, 4..6]
      array.indexes(*params).should == [[1, 2, 3], [2, 3], [5]]
      array.indices(*params).should == [[1, 2, 3], [2, 3], [5]]
    end
  end
end

old, $VERBOSE = $VERBOSE, nil
describe "Array#indexes" do
  it_behaves_like(@array_indexes, :indexes)
end
$VERBOSE = old
