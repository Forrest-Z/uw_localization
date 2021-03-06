#include "MapVisualization.hpp"
#include <uw_localization/maps/node_map.hpp>

namespace vizkit3d {

MapVisualization::~MapVisualization()
{
}


void MapVisualization::updateDataIntern(const uw_localization::Environment& env)
{
    data_env = env;
    updated = true;
}


osg::ref_ptr<osg::Node> MapVisualization::createMainNode()
{
    osg::ref_ptr<osg::Group> root = new osg::Group;
    osg::ref_ptr<osg::Geode> border_geode(new osg::Geode);
    osg::ref_ptr<osg::Geode> landmark_geode(new osg::Geode);
    osg::ref_ptr<osg::Geode> plane_geode(new osg::Geode);
    plane_group = new osg::Group;

    grid = new osg::DrawArrays(osg::PrimitiveSet::LINES, 18, 0);
    border_points = new osg::Vec3Array;
    border_colors = new osg::Vec4Array;
    border_geom = new osg::Geometry;
    border_geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 5));
    border_geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 5, 5));
    border_geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 10, 8));
    border_geom->addPrimitiveSet(grid.get());

    border_geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    border_geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    border_geode->addDrawable(border_geom.get());
    
    plane_points = new osg::Vec3Array;
    plane_colors = new osg::Vec4Array;
    plane_colors->push_back(osg::Vec4(1,1,0,1));

    plane_geom = new osg::Geometry;
    plane_draw_array = new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, plane_points->size());
    plane_geom->addPrimitiveSet(plane_draw_array);
    plane_geom->setColorArray(plane_colors.get());
    plane_geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    plane_geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    plane_geode->addDrawable(plane_geom.get());
 
    landmark_points = new osg::Vec3Array;
    landmark_colors = new osg::Vec4Array;
    landmark_colors->push_back(osg::Vec4(1,1,0,1));

    landmark_geom = new osg::Geometry;
    landmark_geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, 0));
    landmark_geom->setColorArray(landmark_colors.get());
    landmark_geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    landmark_geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    landmark_geode->addDrawable(landmark_geom.get());
    
    root->addChild(border_geode.get());
    root->addChild(plane_geode.get());
    root->addChild(landmark_geode.get());

    setDirty();

    return root;
}


void MapVisualization::updateMainNode(osg::Node* node)
{
    if(updated)
        renderEnvironment(data_env);
}

void MapVisualization::setMap(const QString& p) 
{
    uw_localization::NodeMap m;
    if(!m.fromYaml(p.toAscii().data())){
        std::cerr << "Could not load map " <<  p.toAscii().data() << std::endl;
    }else{
        std::cout << "Map: " << p.toAscii().data() << std::endl;
        data_env = m.getEnvironment();
        updated = true;
    }
}


void MapVisualization::setGridResolution(const int& grid_resolution)
{
    resolution = grid_resolution;

    updated = true;
}



void MapVisualization::renderEnvironment(const uw_localization::Environment& env)
{
    // render borders
    border_points->clear();

    const base::Vector3d& L = env.left_top_corner;
    const base::Vector3d& R = env.right_bottom_corner;

    // top side
    border_points->push_back(osg::Vec3d(L.x(), L.y(), L.z()));
    border_points->push_back(osg::Vec3d(L.x(), R.y(), L.z()));
    border_points->push_back(osg::Vec3d(R.x(), R.y(), L.z()));
    border_points->push_back(osg::Vec3d(R.x(), L.y(), L.z()));
    border_points->push_back(osg::Vec3d(L.x(), L.y(), L.z()));

    // bottom side
    border_points->push_back(osg::Vec3d(L.x(), L.y(), R.z()));
    border_points->push_back(osg::Vec3d(L.x(), R.y(), R.z()));
    border_points->push_back(osg::Vec3d(R.x(), R.y(), R.z()));
    border_points->push_back(osg::Vec3d(R.x(), L.y(), R.z()));
    border_points->push_back(osg::Vec3d(L.x(), L.y(), R.z()));

    // z borders
    border_points->push_back(osg::Vec3d(L.x(), L.y(), R.z()));
    border_points->push_back(osg::Vec3d(L.x(), L.y(), L.z()));

    border_points->push_back(osg::Vec3d(L.x(), R.y(), R.z()));
    border_points->push_back(osg::Vec3d(L.x(), R.y(), L.z()));

    border_points->push_back(osg::Vec3d(R.x(), L.y(), R.z()));
    border_points->push_back(osg::Vec3d(R.x(), L.y(), L.z()));

    border_points->push_back(osg::Vec3d(R.x(), R.y(), R.z()));
    border_points->push_back(osg::Vec3d(R.x(), R.y(), L.z()));

    // set border color to white
    border_colors->push_back(osg::Vec4d(1.0, 1.0, 1.0, 0.5));

    // draw grid lines
    double centre_x = (L.x() + R.x()) * 0.5;
    double centre_y = (L.y() + R.y()) * 0.5;

    double width  = (L.y() - R.y());
    double height = (L.x() - R.x());

    double dx = 0.0;
    double dy = 0.0;

    unsigned grid_vertices = 0;

    /* TODO re-add for grid-visualization
    while( dy <= width * 0.5 ) {
        border_points->push_back(osg::Vec3d(centre_x - height / 2.0, centre_y + dy, R.z()));
        border_points->push_back(osg::Vec3d(centre_x + height / 2.0, centre_y + dy, R.z()));
        border_points->push_back(osg::Vec3d(centre_x - height / 2.0, centre_y - dy, R.z()));
        border_points->push_back(osg::Vec3d(centre_x + height / 2.0, centre_y - dy, R.z()));

        dy += resolution;
        grid_vertices += 4;
    }

    while( dx <= height * 0.5 ) {
        border_points->push_back(osg::Vec3d(centre_x + dx, centre_y - width / 2.0, R.z()));
        border_points->push_back(osg::Vec3d(centre_x + dx, centre_y + width / 2.0, R.z()));

        border_points->push_back(osg::Vec3d(centre_x - dx, centre_y - width / 2.0, R.z()));
        border_points->push_back(osg::Vec3d(centre_x - dx, centre_y + width / 2.0, R.z()));

        dx += resolution;
        grid_vertices += 4;
    }
*/
    
    grid->setCount(grid_vertices);
    
    plane_points->clear();
    plane_colors->clear();
    
/*    for(int i = 0; i < env.planes.size(); i++){
          osg::Vec3d pos(env.planes[i].position.x(), env.planes[i].position.y(), env.planes[i].position.z());
          osg::Vec3d sh(env.planes[i].span_horizontal.x(), env.planes[i].span_horizontal.y(), env.planes[i].span_horizontal.z());
          osg::Vec3d sv(env.planes[i].span_vertical.x(), env.planes[i].span_vertical.y(), env.planes[i].span_vertical.z());
      
      if(base::samples::RigidBodyState::isValidValue(env.planes[i].position) &&
        base::samples::RigidBodyState::isValidValue(env.planes[i].span_horizontal) &&
        base::samples::RigidBodyState::isValidValue(env.planes[i].span_vertical) ){  
            
            plane_points->push_back(pos);
            plane_points->push_back(pos + sh);
            plane_points->push_back(pos + sh + sv);
            plane_points->push_back(pos + sv);          
            plane_colors->push_back(osg::Vec4d(1.0, 1.0, 1.0, 0.3));
            plane_colors->push_back(osg::Vec4d(1.0, 1.0, 1.0, 0.3));
            plane_colors->push_back(osg::Vec4d(1.0, 1.0, 1.0, 0.3));
            plane_colors->push_back(osg::Vec4d(1.0, 1.0, 1.0, 0.3));      
            
      }
      
    }
*/
    border_geom->setColorArray(border_colors.get());
    border_geom->setVertexArray(border_points.get());

    landmark_points->clear();

    std::vector<uw_localization::Landmark>::const_iterator it;
    for(it = env.landmarks.begin(); it != env.landmarks.end(); it++) {
        const int segments = 10;
        for(int i = 0; i < segments; i++) {
            double theta = (double) i / segments * M_PI;
            double x = 0.2 * cos(theta);
            double y = 0.2 * sin(theta);

            osg::Vec3d s(it->point.x() - x, it->point.y() - y, it->point.z());
            osg::Vec3d p(it->point.x() + x, it->point.y() + y, it->point.z());

            landmark_points->push_back(s);
            landmark_points->push_back(p);

            std::cout << "draw landmark" << std::endl;
        }
    }
    
    plane_draw_array->setCount(plane_points->size());
    plane_geom->setColorArray(plane_colors.get());
    plane_geom->setVertexArray(border_points.get());

    dynamic_cast<osg::DrawArrays*>(landmark_geom->getPrimitiveSet(0))->setCount(landmark_points->size());
    landmark_geom->setVertexArray(landmark_points.get());
    
    updated = false;
}




void MapVisualization::updatePlaneNode(osg::Geode* geode, const uw_localization::Plane& plane)
{
    osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode->getDrawable(0));
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array; 
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;

    osg::Vec3d pos(plane.position.x(), plane.position.y(), plane.position.z());
    osg::Vec3d sh(plane.span_horizontal.x(), plane.span_horizontal.y(), plane.span_horizontal.z());
    osg::Vec3d sv(plane.span_vertical.x(), plane.span_vertical.y(), plane.span_vertical.z());
    
    if(base::samples::RigidBodyState::isValidValue(plane.position) &&
      base::samples::RigidBodyState::isValidValue(plane.span_horizontal) &&
      base::samples::RigidBodyState::isValidValue(plane.span_vertical) ){
    
        vertices->push_back(pos);
        vertices->push_back(pos + sh);
        vertices->push_back(pos + sh + sv);
        vertices->push_back(pos + sv);

        colors->push_back(osg::Vec4d(1.0, 1.0, 1.0, 0.3));

        geom->setVertexArray(vertices.get());
        geom->setColorArray(colors.get());
      }
      else{
        std::cout << "Detected nan" << std::endl;
      }
}


//VizkitQtPlugin(MapVisualization)
}

