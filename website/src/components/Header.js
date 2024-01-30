import React from "react";
import { Container, Col, Nav } from "react-bootstrap";
import { useStaticQuery, graphql } from "gatsby";
import StarCount from "./widgets/StarCount";
import Download from "./widgets/Download";
import CriblSiteNav from "./criblSiteNav";
import logo from "../images/logo-appview.svg";
import "../scss/_appViewNav.scss";

export default function Header() {
  const data = useStaticQuery(graphql`
    query SiteNavQuery {
      allHeaderYaml {
        edges {
          node {
            name
            path
          }
        }
      }
    }
  `);

  return (
    <nav>
      <CriblSiteNav />
      <Container
        fluid
        className="nav-container"
        id="appviewNav"
        style={{ backgroundColor: "#fff !important" }}
      >
        <Col>
          <Nav>
            <Nav.Item className="branding">
              <Nav.Link href="/">
                <img
                  className="appview-logo"
                  src={logo}
                  alt="AppView"
                  width="100"
                />
              </Nav.Link>
            </Nav.Item>
            {data.allHeaderYaml.edges.map((item, i) => {
              return (
                <Nav.Item key={i}>
                  <Nav.Link
                    href={item.node.path}
                    activestyle={{ color: "#FD6600", fontWeight: 700 }}
                  >
                    {item.node.name}{" "}
                  </Nav.Link>
                </Nav.Item>
              );
            })}
          </Nav>
        </Col>
        <Col className="appview-nav-widgets">
          <StarCount />
          <Download btnText="Download" />
        </Col>
      </Container>
    </nav>
  );
}
