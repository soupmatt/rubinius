require File.dirname(__FILE__) + '/../../spec_helper'
require File.dirname(__FILE__) + '/fixtures/classes'
require File.dirname(__FILE__) + '/indexes_spec'

old, $VERBOSE = $VERBOSE, nil
describe "Array#indices" do
  it_behaves_like(@array_indexes, :indices)
end
$VERBOSE = old
